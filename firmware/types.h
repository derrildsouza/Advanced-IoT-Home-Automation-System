#pragma once

#include <Arduino.h>

// Source of the relay switch trigger
enum class CommandSource : uint8_t {
    PHYSICAL_BUTTON = 0,
    IR_REMOTE,
    WEB_UI,
    REST_API,
    MQTT,
    BOOT_RESTORE
};

// Queue message for commanding relay changes
struct RelayCommand_t {
    uint8_t channel;          // 0 to 3 (for relays 1 to 4)
    int8_t targetState;       // 0: OFF, 1: ON, -1: TOGGLE
    CommandSource source;
};

// Real-time snapshot of the system
struct SystemState_t {
    bool relayStates[4];      // Relay 1..4 current states (true=ON, false=OFF)
    uint16_t ldrRaw;          // Raw 12-bit ADC value (0 - 4095)
    uint8_t ledBrightness;    // Current calculated LED brightness (0 - 255)
    bool wifiConnected;
    bool mqttConnected;
    uint32_t uptimeSeconds;
    uint32_t lastIRCode;      // Most recent decoded IR code (hex format)
};
