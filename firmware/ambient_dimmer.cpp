#include "ambient_dimmer.h"
#include "config.h"

const uint8_t AmbientDimmer::ledPins[4] = {
    PIN_LED_1,
    PIN_LED_2,
    PIN_LED_3,
    PIN_LED_4
};

const uint8_t AmbientDimmer::ledcChannels[4] = {
    LEDC_CH_LED1,
    LEDC_CH_LED2,
    LEDC_CH_LED3,
    LEDC_CH_LED4
};

AmbientDimmer ambientDimmer;

AmbientDimmer::AmbientDimmer() :
    rawLdr(2048),
    filteredLdr(2048.0f),
    currentBrightness(128),
    autoDimmingEnabled(true),
    lastSampleTime(0)
{
}

void AmbientDimmer::begin() {
    // 1. Configure ADC
    analogReadResolution(12); // 0 - 4095
    pinMode(PIN_LDR_ADC, INPUT);

    // 2. Configure 4 hardware LEDC PWM channels
    for (int i = 0; i < 4; i++) {
        ledcSetup(ledcChannels[i], LEDC_PWM_FREQ, LEDC_PWM_RES);
        ledcAttachPin(ledPins[i], ledcChannels[i]);
        ledcWrite(ledcChannels[i], 0); // Start OFF
    }
}

void AmbientDimmer::setManualBrightness(uint8_t val) {
    autoDimmingEnabled = false;
    currentBrightness = val;
}

void AmbientDimmer::update(const bool relayStates[4]) {
    uint32_t now = millis();
    if (now - lastSampleTime >= LDR_SAMPLE_INTERVAL_MS) {
        lastSampleTime = now;

        if (autoDimmingEnabled) {
            // Read ADC1 Channel 6 (GPIO 34)
            rawLdr = analogRead(PIN_LDR_ADC);

            // Apply Exponential Moving Average (EMA) to smooth out fluctuations
            filteredLdr = (LDR_EMA_ALPHA * (float)rawLdr) + ((1.0f - LDR_EMA_ALPHA) * filteredLdr);

            // In our voltage divider:
            // 3.3V ---[ 10k ]---+---[ LDR ]--- GND
            // Bright light: LDR resistance is LOW (~1k) -> Voltage is LOW (ADC ~ 300)
            // Darkness:     LDR resistance is HIGH (>500k) -> Voltage is HIGH (ADC ~ 3800)
            //
            // When it's DARK (high ADC reading), we want the LEDs to be DIM (MIN_LED_BRIGHTNESS)
            // When it's BRIGHT (low ADC reading), we want the LEDs to be BRIGHT (MAX_LED_BRIGHTNESS)
            
            float norm = (filteredLdr - 300.0f) / (3800.0f - 300.0f);
            if (norm < 0.0f) norm = 0.0f;
            if (norm > 1.0f) norm = 1.0f;

            // Invert: darkness (norm ~ 1.0) -> min brightness; daylight (norm ~ 0.0) -> max brightness
            float targetDuty = MAX_LED_BRIGHTNESS - (norm * (MAX_LED_BRIGHTNESS - MIN_LED_BRIGHTNESS));
            currentBrightness = (uint8_t)targetDuty;
        }

        updatePWMOutputs(relayStates);
    }
}

void AmbientDimmer::updatePWMOutputs(const bool relayStates[4]) {
    for (int i = 0; i < 4; i++) {
        if (relayStates[i]) {
            // Circuit is active: set LED duty cycle according to ambient brightness
            ledcWrite(ledcChannels[i], currentBrightness);
        } else {
            // Circuit is inactive: turn LED off
            ledcWrite(ledcChannels[i], 0);
        }
    }
}
