#include "ambient_dimmer.h"
#include "config.h"

const uint8_t AmbientDimmer::statusLedPins[4] = {
    PIN_LED_1,
    PIN_LED_2,
    PIN_LED_3,
    PIN_LED_4
};

const uint8_t AmbientDimmer::statusLedcChannels[4] = {
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
    // 1. Configure ADC for LDR
    analogReadResolution(12); // 0 - 4095
    pinMode(PIN_LDR_ADC, INPUT);

    // 2. Configure 4 hardware LEDC PWM channels for Relay Status LEDs
    for (int i = 0; i < 4; i++) {
        ledcSetup(statusLedcChannels[i], LEDC_PWM_FREQ, LEDC_PWM_RES);
        ledcAttachPin(statusLedPins[i], statusLedcChannels[i]);
        ledcWrite(statusLedcChannels[i], 0); // Start OFF
    }

    // 3. Configure Power LED channel (GPIO 12 - LEDC Ch 4)
    ledcSetup(LEDC_CH_PWR, LEDC_PWM_FREQ, LEDC_PWM_RES);
    ledcAttachPin(PIN_POWER_LED, LEDC_CH_PWR);
    ledcWrite(LEDC_CH_PWR, currentBrightness); // Start ON

    // 4. Configure Wi-Fi Status LED channel (GPIO 2 - LEDC Ch 5)
    ledcSetup(LEDC_CH_WIFI, LEDC_PWM_FREQ, LEDC_PWM_RES);
    ledcAttachPin(PIN_WIFI_LED, LEDC_CH_WIFI);
    ledcWrite(LEDC_CH_WIFI, 0); // Start OFF until network connects
}

void AmbientDimmer::setManualBrightness(uint8_t val) {
    autoDimmingEnabled = false;
    currentBrightness = val;
}

void AmbientDimmer::update(const bool relayStates[4], bool wifiConnected, bool wifiAPMode) {
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

        updatePWMOutputs(relayStates, wifiConnected, wifiAPMode);
    }
}

void AmbientDimmer::updatePWMOutputs(const bool relayStates[4], bool wifiConnected, bool wifiAPMode) {
    // 1. Update 4 Relay status LEDs (Channel mirrors)
    for (int i = 0; i < 4; i++) {
        if (relayStates[i]) {
            ledcWrite(statusLedcChannels[i], currentBrightness);
        } else {
            ledcWrite(statusLedcChannels[i], 0);
        }
    }

    // 2. Update Power Indicator LED (Always ON, auto-dimmed based on ambient lux)
    ledcWrite(LEDC_CH_PWR, currentBrightness);

    // 3. Update Wi-Fi Status LED (Auto-dimmed peak brightness)
    if (wifiConnected) {
        // Solid ON at current ambient brightness level
        ledcWrite(LEDC_CH_WIFI, currentBrightness);
    } else if (wifiAPMode) {
        // Soft pulsing / breathing effect during AP configuration mode
        float phase = (float)(millis() % 1200) / 1200.0f;
        float pulse = (sin(phase * 2.0f * 3.14159265f) + 1.0f) * 0.5f;
        uint8_t duty = (uint8_t)(pulse * (float)currentBrightness);
        ledcWrite(LEDC_CH_WIFI, duty);
    } else {
        // Connecting: gentle 500ms blink scaled by current ambient brightness
        bool blink = ((millis() / 250) % 2) == 0;
        ledcWrite(LEDC_CH_WIFI, blink ? currentBrightness : 0);
    }
}
