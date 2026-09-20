#include "ir_controller.h"
#include "config.h"

IRController irCtrl;

IRController::IRController() :
    irRecv(PIN_IR_RECV),
    commandCallback(nullptr),
    lastCode(0)
{
    keyMappings[0] = DEFAULT_IR_KEY_CH1;
    keyMappings[1] = DEFAULT_IR_KEY_CH2;
    keyMappings[2] = DEFAULT_IR_KEY_CH3;
    keyMappings[3] = DEFAULT_IR_KEY_CH4;
    keyMappings[4] = DEFAULT_IR_KEY_ALL_OFF;
}

void IRController::begin(IRCommandCallback_t onIRCommand) {
    commandCallback = onIRCommand;
    prefs.begin("ir_keys", false);
    loadKeyMappings();
    irRecv.enableIRIn(); // Start the receiver
}

void IRController::loadKeyMappings() {
    char key[8];
    for (int i = 0; i < 5; i++) {
        snprintf(key, sizeof(key), "k%d", i);
        keyMappings[i] = prefs.getUInt(key, keyMappings[i]);
    }
}

void IRController::setCustomKey(uint8_t channel, uint32_t hexCode) {
    if (channel > 4) return;
    keyMappings[channel] = hexCode;
    char key[8];
    snprintf(key, sizeof(key), "k%d", channel);
    prefs.putUInt(key, hexCode);
}

uint32_t IRController::getKey(uint8_t channel) const {
    if (channel > 4) return 0;
    return keyMappings[channel];
}

void IRController::update() {
    if (irRecv.decode(&results)) {
        // Filter out repeat codes (0xFFFFFFFF in NEC)
        if (results.value != 0xFFFFFFFF && results.value != 0) {
            lastCode = results.value;

            // Match against stored keys
            if (commandCallback) {
                if (lastCode == keyMappings[0]) {
                    commandCallback(0);
                } else if (lastCode == keyMappings[1]) {
                    commandCallback(1);
                } else if (lastCode == keyMappings[2]) {
                    commandCallback(2);
                } else if (lastCode == keyMappings[3]) {
                    commandCallback(3);
                } else if (lastCode == keyMappings[4]) {
                    commandCallback(-1); // ALL OFF
                }
            }
        }
        irRecv.resume(); // Receive the next value
    }
}
