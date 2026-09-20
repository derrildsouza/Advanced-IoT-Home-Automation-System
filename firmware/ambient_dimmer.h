#pragma once

#include <Arduino.h>
#include "pins.h"
#include "types.h"

class AmbientDimmer {
public:
    AmbientDimmer();
    void begin();
    void update(const bool relayStates[4]); // Called periodically from Core 1
    
    uint16_t getRawLdr() const { return rawLdr; }
    uint8_t getCalculatedBrightness() const { return currentBrightness; }
    void setAutoDimming(bool enable) { autoDimmingEnabled = enable; }
    bool isAutoDimming() const { return autoDimmingEnabled; }
    void setManualBrightness(uint8_t val);

private:
    static const uint8_t ledPins[4];
    static const uint8_t ledcChannels[4];
    
    uint16_t rawLdr;
    float filteredLdr;
    uint8_t currentBrightness;
    bool autoDimmingEnabled;
    uint32_t lastSampleTime;

    void updatePWMOutputs(const bool relayStates[4]);
};

extern AmbientDimmer ambientDimmer;
