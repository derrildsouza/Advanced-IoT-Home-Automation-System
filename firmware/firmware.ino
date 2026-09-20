#include <Arduino.h>
#include "pins.h"
#include "types.h"
#include "config.h"
#include "relay_controller.h"
#include "button_matrix.h"
#include "ir_controller.h"
#include "ambient_dimmer.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "mqtt_client.h"

// FreeRTOS Queues
static QueueHandle_t commandQueue = nullptr;
static QueueHandle_t eventQueue = nullptr;

// Task Handles
TaskHandle_t hardwareTaskHandle = nullptr;
TaskHandle_t networkTaskHandle = nullptr;

// =============================================================================
// Hardware Callbacks (Core 1)
// =============================================================================

// Called immediately when a tactile button is physically pressed
void onButtonPress(uint8_t channel) {
    RelayCommand_t cmd;
    cmd.channel = channel;
    cmd.targetState = -1; // Toggle
    cmd.source = CommandSource::PHYSICAL_BUTTON;

    // Zero latency: Execute immediately on Core 1
    relayCtrl.toggleRelay(channel);

    // Notify network task via event queue
    if (eventQueue) {
        xQueueSend(eventQueue, &cmd, 0);
    }
}

// Reset button handler (short press = reboot; long press 5s = AP mode)
void onResetPress(bool longPress) {
    if (longPress) {
        Serial.println("[SYSTEM] Long-press detected: Entering Wi-Fi AP Mode...");
        wifiMgr.startAP();
    } else {
        Serial.println("[SYSTEM] Short-press detected: Rebooting ESP32...");
        delay(200);
        ESP.restart();
    }
}

// IR Remote command decoded callback
void onIRCommand(int8_t channel) {
    if (channel >= 0 && channel < 4) {
        relayCtrl.toggleRelay(channel);
        if (eventQueue) {
            RelayCommand_t cmd = { (uint8_t)channel, -1, CommandSource::IR_REMOTE };
            xQueueSend(eventQueue, &cmd, 0);
        }
    } else if (channel == -1) {
        // All OFF master key
        relayCtrl.setAll(false);
        for (uint8_t i = 0; i < 4; i++) {
            if (eventQueue) {
                RelayCommand_t cmd = { i, 0, CommandSource::IR_REMOTE };
                xQueueSend(eventQueue, &cmd, 0);
            }
        }
    }
}

// =============================================================================
// Core 1 Task: Hardware Real-Time Engine
// =============================================================================
void vHardwareTask(void *pvParameters) {
    Serial.println("[Core 1] Hardware Engine Started");

    // Initialize all hardware drivers
    relayCtrl.begin();
    buttonMatrix.begin(onButtonPress, onResetPress);
    irCtrl.begin(onIRCommand);
    ambientDimmer.begin();

    // Broadcast initial state on boot
    for (uint8_t i = 0; i < 4; i++) {
        if (eventQueue) {
            RelayCommand_t cmd = { i, (int8_t)(relayCtrl.getState(i) ? 1 : 0), CommandSource::BOOT_RESTORE };
            xQueueSend(eventQueue, &cmd, 0);
        }
    }

    for (;;) {
        // 1. Process physical button debouncing
        buttonMatrix.update();

        // 2. Decode incoming IR pulses
        irCtrl.update();

        // 3. Process remote command requests from Core 0 (Web, REST, MQTT)
        RelayCommand_t cmd;
        if (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {
            bool changed = false;
            if (cmd.targetState == -1) {
                changed = relayCtrl.toggleRelay(cmd.channel);
            } else {
                changed = relayCtrl.setRelay(cmd.channel, cmd.targetState == 1);
            }

            if (changed && eventQueue) {
                xQueueSend(eventQueue, &cmd, 0);
            }
        }

        // 4. Update status LEDs and ambient auto-dimming
        bool currentStates[4];
        for (int i = 0; i < 4; i++) currentStates[i] = relayCtrl.getState(i);
        ambientDimmer.update(currentStates);

        // Deterministic tick pause
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// =============================================================================
// Core 0 Task: Network, Web, & MQTT Engine
// =============================================================================
void vNetworkTask(void *pvParameters) {
    Serial.println("[Core 0] Network Engine Started");

    wifiMgr.begin();
    webServerCtrl.begin(commandQueue);
    mqttManager.begin(commandQueue);

    uint32_t lastTelemetryTime = 0;

    for (;;) {
        wifiMgr.update();
        webServerCtrl.update();
        mqttManager.update();

        // Check for state changes from Core 1
        RelayCommand_t event;
        while (xQueueReceive(eventQueue, &event, 0) == pdTRUE) {
            bool state = relayCtrl.getState(event.channel);
            // Push to MQTT
            mqttManager.publishRelayState(event.channel, state);

            // Push to WebSockets
            SystemState_t sysState;
            for (int i = 0; i < 4; i++) sysState.relayStates[i] = relayCtrl.getState(i);
            sysState.ldrRaw = ambientDimmer.getRawLdr();
            sysState.ledBrightness = ambientDimmer.getCalculatedBrightness();
            sysState.uptimeSeconds = millis() / 1000;
            sysState.lastIRCode = irCtrl.getLastCode();
            webServerCtrl.broadcastState(sysState);
        }

        // Periodic telemetry broadcast every 2.5 seconds
        uint32_t now = millis();
        if (now - lastTelemetryTime >= 2500) {
            lastTelemetryTime = now;
            SystemState_t sysState;
            for (int i = 0; i < 4; i++) sysState.relayStates[i] = relayCtrl.getState(i);
            sysState.ldrRaw = ambientDimmer.getRawLdr();
            sysState.ledBrightness = ambientDimmer.getCalculatedBrightness();
            sysState.uptimeSeconds = millis() / 1000;
            sysState.lastIRCode = irCtrl.getLastCode();

            webServerCtrl.broadcastState(sysState);
            mqttManager.publishFullStatus(sysState);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// =============================================================================
// Arduino Setup & Loop
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n==========================================");
    Serial.println("  Advanced IoT Home Automation System");
    Serial.printf("  Firmware Version: %s\n", FIRMWARE_VERSION);
    Serial.println("==========================================");

    // Create FreeRTOS Queues
    commandQueue = xQueueCreate(QUEUE_SIZE_COMMANDS, sizeof(RelayCommand_t));
    eventQueue = xQueueCreate(QUEUE_SIZE_EVENTS, sizeof(RelayCommand_t));

    if (!commandQueue || !eventQueue) {
        Serial.println("[ERROR] Failed to allocate FreeRTOS queues!");
        while (1) delay(1000);
    }

    // Pin tasks to dedicated CPU cores
    // Core 1: Real-time hardware switching, buttons, IR, PWM
    xTaskCreatePinnedToCore(
        vHardwareTask,
        "HardwareTask",
        TASK_STACK_HARDWARE,
        NULL,
        2, // High priority
        &hardwareTaskHandle,
        1  // Core 1
    );

    // Core 0: Asynchronous networking, HTTP, WebSockets, MQTT
    xTaskCreatePinnedToCore(
        vNetworkTask,
        "NetworkTask",
        TASK_STACK_NETWORK,
        NULL,
        1, // Normal priority
        &networkTaskHandle,
        0  // Core 0
    );
}

void loop() {
    // Both engines run inside dedicated FreeRTOS tasks pinned to cores.
    // Nothing required in default loop.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
