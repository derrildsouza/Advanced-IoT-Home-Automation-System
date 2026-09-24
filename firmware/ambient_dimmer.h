#pragma once

#include <Arduino.h>
#include "pins.h"
#include "types.h"

class AmbientDimmer {
public:
    AmbientDimmer();
    void begin();
    void update(const bool relayStates[4], bool wifiConnected = false, bool wifiAPMode = false); // Called periodically from Core 1
    
    uint16_t getRawLdr() const { return rawLdr; }
    uint8_t getCalculatedBrightness() const { return currentBrightness; }
    void setAutoDimming(bool enable) { autoDimmingEnabled = enable; }
    bool isAutoDimming() const { return autoDimmingEnabled; }
    void setManualBrightness(uint8_t val);

private:
    static const uint8_t statusLedPins[4];
    static const uint8_t statusLedcChannels[4];
    
    uint16_t rawLdr;
    float filteredLdr;
    uint8_t currentBrightness;
    bool autoDimmingEnabled;
    uint32_t lastSampleTime;

    void updatePWMOutputs(const bool relayStates[4], bool wifiConnected, bool wifiAPMode);
};

extern AmbientDimmer ambientDimmer;
