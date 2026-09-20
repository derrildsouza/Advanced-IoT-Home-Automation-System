#include "mqtt_client.h"
#include "config.h"
#include "relay_controller.h"
#include <ArduinoJson.h>

SmartMQTTClient mqttManager;

SmartMQTTClient::SmartMQTTClient() :
    mqttClient(espClient),
    commandQueue(nullptr),
    lastReconnectAttempt(0),
    haDiscoverySent(false)
{
}

void SmartMQTTClient::mqttCallbackWrapper(char* topic, byte* payload, unsigned int length) {
    mqttManager.handleMessage(topic, payload, length);
}

void SmartMQTTClient::begin(QueueHandle_t cmdQueue) {
    commandQueue = cmdQueue;
    mqttClient.setServer(DEFAULT_MQTT_BROKER, DEFAULT_MQTT_PORT);
    mqttClient.setCallback(mqttCallbackWrapper);
    mqttClient.setBufferSize(512); // Buffer size for HA discovery JSON payloads
}

void SmartMQTTClient::reconnect() {
    if (mqttClient.connected()) return;

    uint32_t now = millis();
    if (now - lastReconnectAttempt >= 5000) { // Retry every 5s without blocking
        lastReconnectAttempt = now;
        Serial.print("[MQTT] Connecting to broker...");

        // Connect with Last Will and Testament (LWT)
        bool success = false;
        if (strlen(DEFAULT_MQTT_USER) > 0) {
            success = mqttClient.connect(MQTT_CLIENT_ID, DEFAULT_MQTT_USER, DEFAULT_MQTT_PASS,
                                         "home/switch/availability", 1, true, "offline");
        } else {
            success = mqttClient.connect(MQTT_CLIENT_ID,
                                         "home/switch/availability", 1, true, "offline");
        }

        if (success) {
            Serial.println(" Connected!");
            mqttClient.publish("home/switch/availability", "online", true);

            // Subscribe to relay command topics
            mqttClient.subscribe("home/switch/relay1/set");
            mqttClient.subscribe("home/switch/relay2/set");
            mqttClient.subscribe("home/switch/relay3/set");
            mqttClient.subscribe("home/switch/relay4/set");
            mqttClient.subscribe("home/switch/all/set");

            // Publish initial states
            for (uint8_t i = 0; i < 4; i++) {
                publishRelayState(i, relayCtrl.getState(i));
            }

            // Publish Home Assistant auto-discovery
            publishHADiscovery();
        } else {
            Serial.printf(" Failed (rc=%d)\n", mqttClient.state());
        }
    }
}

void SmartMQTTClient::publishRelayState(uint8_t channel, bool state) {
    if (!mqttClient.connected() || channel >= 4) return;
    char topic[32];
    snprintf(topic, sizeof(topic), "home/switch/relay%d/state", channel + 1);
    mqttClient.publish(topic, state ? "ON" : "OFF", true);
}

void SmartMQTTClient::publishFullStatus(const SystemState_t& state) {
    if (!mqttClient.connected()) return;

    JsonDocument doc;
    JsonArray arr = doc["relays"].to<JsonArray>();
    for (int i = 0; i < 4; i++) {
        arr.add(state.relayStates[i]);
    }
    doc["ldr"] = state.ldrRaw;
    doc["led_brightness"] = state.ledBrightness;
    doc["uptime"] = state.uptimeSeconds;

    char buffer[256];
    serializeJson(doc, buffer);
    mqttClient.publish(MQTT_TOPIC_STATUS, buffer);
}

void SmartMQTTClient::publishHADiscovery() {
    if (!mqttClient.connected()) return;

    for (int i = 0; i < 4; i++) {
        char discTopic[80];
        snprintf(discTopic, sizeof(discTopic), "%s/switch/smartswitch_relay%d/config", HA_DISCOVERY_PREFIX, i + 1);

        char stateTopic[40];
        snprintf(stateTopic, sizeof(stateTopic), "home/switch/relay%d/state", i + 1);
        char cmdTopic[40];
        snprintf(cmdTopic, sizeof(cmdTopic), "home/switch/relay%d/set", i + 1);

        JsonDocument doc;
        char nameBuf[32];
        snprintf(nameBuf, sizeof(nameBuf), "Appliance %d", i + 1);
        doc["name"] = nameBuf;
        doc["stat_t"] = stateTopic;
        doc["cmd_t"] = cmdTopic;
        doc["pl_on"] = "ON";
        doc["pl_off"] = "OFF";
        doc["avty_t"] = "home/switch/availability";

        JsonObject dev = doc["dev"].to<JsonObject>();
        JsonArray ids = dev["ids"].to<JsonArray>();
        ids.add("smartswitch_esp32_01");
        dev["name"] = "Smart Switch 4CH";
        dev["mf"] = "Antigravity";
        dev["mdl"] = "ESP32-4CH-RELAY";
        dev["sw"] = FIRMWARE_VERSION;

        char buffer[384];
        serializeJson(doc, buffer);
        mqttClient.publish(discTopic, buffer, true);
    }
}

void SmartMQTTClient::handleMessage(char* topic, byte* payload, unsigned int length) {
    char message[32];
    if (length >= sizeof(message)) length = sizeof(message) - 1;
    memcpy(message, payload, length);
    message[length] = '\0';

    String topStr = String(topic);
    String msgStr = String(message);
    msgStr.toUpperCase();

    // Check individual relay command topics
    for (uint8_t i = 0; i < 4; i++) {
        char expected[32];
        snprintf(expected, sizeof(expected), "home/switch/relay%d/set", i + 1);
        if (topStr == expected) {
            int8_t target = -1;
            if (msgStr == "ON" || msgStr == "1") target = 1;
            else if (msgStr == "OFF" || msgStr == "0") target = 0;
            else if (msgStr == "TOGGLE") target = -1;

            if (commandQueue) {
                RelayCommand_t cmd = { i, target, CommandSource::MQTT };
                xQueueSend(commandQueue, &cmd, 0);
            }
            return;
        }
    }

    // Check "all" command topic
    if (topStr == "home/switch/all/set") {
        bool state = (msgStr == "ON" || msgStr == "1");
        for (uint8_t i = 0; i < 4; i++) {
            if (commandQueue) {
                RelayCommand_t cmd = { i, (int8_t)(state ? 1 : 0), CommandSource::MQTT };
                xQueueSend(commandQueue, &cmd, 0);
            }
        }
    }
}

void SmartMQTTClient::update() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
            reconnect();
        } else {
            mqttClient.loop();
        }
    }
}
