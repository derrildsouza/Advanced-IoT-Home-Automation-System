#pragma once

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Preferences.h>
#include "pins.h"
#include "types.h"

typedef void (*IRCommandCallback_t)(int8_t channel); // 0-3 for relay, -1 for all-off

class IRController {
public:
    IRController();
    void begin(IRCommandCallback_t onIRCommand);
    void update(); // Called from Core 1 hardware task loop

    uint32_t getLastCode() const { return lastCode; }
    void setCustomKey(uint8_t channel, uint32_t hexCode);
    uint32_t getKey(uint8_t channel) const;

private:
    IRrecv irRecv;
    decode_results results;
    IRCommandCallback_t commandCallback;
    uint32_t lastCode;
    uint32_t keyMappings[5]; // 0-3: Ch1-4, 4: ALL_OFF
    Preferences prefs;

    void loadKeyMappings();
};

extern IRController irCtrl;
