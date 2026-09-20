#include "button_matrix.h"
#include "config.h"

const uint8_t ButtonMatrix::buttonPins[4] = {
    PIN_BUTTON_1,
    PIN_BUTTON_2,
    PIN_BUTTON_3,
    PIN_BUTTON_4
};

ButtonMatrix buttonMatrix;

ButtonMatrix::ButtonMatrix() :
    buttonCallback(nullptr),
    resetCallback(nullptr),
    lastResetSteadyState(HIGH),
    lastResetFlickerState(HIGH),
    lastResetDebounceTime(0),
    resetPressStartTime(0),
    resetHandled(false)
{
    for (int i = 0; i < 4; i++) {
        lastSteadyState[i] = HIGH;
        lastFlickerState[i] = HIGH;
        lastDebounceTime[i] = 0;
    }
}

void ButtonMatrix::begin(ButtonCallback_t onButtonPress, ResetButtonCallback_t onResetPress) {
    buttonCallback = onButtonPress;
    resetCallback = onResetPress;

    // 4 Tactile switches with internal pull-up (active LOW)
    for (int i = 0; i < 4; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        lastSteadyState[i] = digitalRead(buttonPins[i]);
        lastFlickerState[i] = lastSteadyState[i];
    }

    // Reset pin: use internal pull-up (active LOW when button pressed)
    pinMode(PIN_CONFIG_RESET, INPUT_PULLUP);
    lastResetSteadyState = digitalRead(PIN_CONFIG_RESET);
    lastResetFlickerState = lastResetSteadyState;
    resetHandled = (lastResetSteadyState == LOW); // Ignore if held/low during boot
}

void ButtonMatrix::update() {
    uint32_t now = millis();

    // 1. Debounce and check the 4 tactile relay override buttons
    for (uint8_t i = 0; i < 4; i++) {
        bool reading = digitalRead(buttonPins[i]);

        if (reading != lastFlickerState[i]) {
            lastDebounceTime[i] = now;
            lastFlickerState[i] = reading;
        }

        if ((now - lastDebounceTime[i]) >= BUTTON_DEBOUNCE_MS) {
            // State has stabilized
            if (lastSteadyState[i] == HIGH && reading == LOW) {
                // Falling edge detected (button pressed down)
                if (buttonCallback) {
                    buttonCallback(i);
                }
            }
            lastSteadyState[i] = reading;
        }
    }

    // 2. Debounce and check the external reset/config button
    bool resetReading = digitalRead(PIN_CONFIG_RESET);
    if (resetReading != lastResetFlickerState) {
        lastResetDebounceTime = now;
        lastResetFlickerState = resetReading;
    }

    if ((now - lastResetDebounceTime) >= BUTTON_DEBOUNCE_MS) {
        if (lastResetSteadyState == HIGH && resetReading == LOW) {
            // Button just pressed down
            resetPressStartTime = now;
            resetHandled = false;
        } else if (lastResetSteadyState == LOW && resetReading == LOW) {
            // Button is being held
            if (!resetHandled && (now - resetPressStartTime >= 5000)) {
                // Held for more than 5 seconds -> Wi-Fi AP provisioning
                resetHandled = true;
                if (resetCallback) {
                    resetCallback(true); // longPress = true
                }
            }
        } else if (lastResetSteadyState == LOW && resetReading == HIGH) {
            // Button released
            if (!resetHandled) {
                // Short press -> trigger regular reboot
                if (resetCallback) {
                    resetCallback(false); // longPress = false
                }
            }
            resetHandled = false;
        }
        lastResetSteadyState = resetReading;
    }
}
