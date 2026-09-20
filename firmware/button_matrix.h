#pragma once

#include <Arduino.h>
#include "pins.h"
#include "types.h"

typedef void (*ButtonCallback_t)(uint8_t channel);
typedef void (*ResetButtonCallback_t)(bool longPress);

class ButtonMatrix {
public:
    ButtonMatrix();
    void begin(ButtonCallback_t onButtonPress, ResetButtonCallback_t onResetPress);
    void update(); // Called frequently from Core 1 hardware task loop

private:
    static const uint8_t buttonPins[4];
    ButtonCallback_t buttonCallback;
    ResetButtonCallback_t resetCallback;

    // Per-button debounce state
    bool lastSteadyState[4];
    bool lastFlickerState[4];
    uint32_t lastDebounceTime[4];

    // Reset button state
    bool lastResetSteadyState;
    bool lastResetFlickerState;
    uint32_t lastResetDebounceTime;
    uint32_t resetPressStartTime;
    bool resetHandled;
};

extern ButtonMatrix buttonMatrix;
