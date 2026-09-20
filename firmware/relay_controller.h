#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "pins.h"
#include "types.h"

class RelayController {
public:
    RelayController();
    void begin();
    
    // Channel operations (channel 0 to 3)
    bool setRelay(uint8_t channel, bool state, bool persist = true);
    bool toggleRelay(uint8_t channel);
    void setAll(bool state);
    
    // Status queries
    bool getState(uint8_t channel) const;
    uint8_t getStateMask() const;

private:
    static const uint8_t relayPins[4];
    bool states[4];
    Preferences prefs;
    
    void applyHardwarePin(uint8_t channel, bool state);
    void saveState(uint8_t channel, bool state);
    void loadSavedStates();
};

extern RelayController relayCtrl;
