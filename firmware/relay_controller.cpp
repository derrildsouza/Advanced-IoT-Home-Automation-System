#include "relay_controller.h"

const uint8_t RelayController::relayPins[4] = {
    PIN_RELAY_1,
    PIN_RELAY_2,
    PIN_RELAY_3,
    PIN_RELAY_4
};

RelayController relayCtrl;

RelayController::RelayController() {
    for (int i = 0; i < 4; i++) {
        states[i] = false;
    }
}

void RelayController::begin() {
    // 1. Initialize hardware pins to INACTIVE before setting as OUTPUT to avoid glitches
    for (int i = 0; i < 4; i++) {
        digitalWrite(relayPins[i], RELAY_INACTIVE_LEVEL);
        pinMode(relayPins[i], OUTPUT);
    }
    
    // 2. Open NVS Preferences namespace
    prefs.begin("relays", false);
    
    // 3. Load previous states (Option A: Restore after reboot)
    loadSavedStates();
    
    // 4. Apply loaded states to hardware
    for (int i = 0; i < 4; i++) {
        applyHardwarePin(i, states[i]);
    }
}

void RelayController::loadSavedStates() {
    char key[8];
    for (int i = 0; i < 4; i++) {
        snprintf(key, sizeof(key), "ch%d", i);
        states[i] = prefs.getBool(key, false); // Default to false if not previously stored
    }
}

void RelayController::saveState(uint8_t channel, bool state) {
    if (channel >= 4) return;
    char key[8];
    snprintf(key, sizeof(key), "ch%d", channel);
    prefs.putBool(key, state);
}

void RelayController::applyHardwarePin(uint8_t channel, bool state) {
    if (channel >= 4) return;
    digitalWrite(relayPins[channel], state ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
}

bool RelayController::setRelay(uint8_t channel, bool state, bool persist) {
    if (channel >= 4) return false;
    if (states[channel] == state) return true; // No change needed
    
    states[channel] = state;
    applyHardwarePin(channel, state);
    
    if (persist) {
        saveState(channel, state);
    }
    return true;
}

bool RelayController::toggleRelay(uint8_t channel) {
    if (channel >= 4) return false;
    return setRelay(channel, !states[channel], true);
}

void RelayController::setAll(bool state) {
    for (uint8_t i = 0; i < 4; i++) {
        setRelay(i, state, true);
    }
}

bool RelayController::getState(uint8_t channel) const {
    if (channel >= 4) return false;
    return states[channel];
}

uint8_t RelayController::getStateMask() const {
    uint8_t mask = 0;
    for (int i = 0; i < 4; i++) {
        if (states[i]) {
            mask |= (1 << i);
        }
    }
    return mask;
}
