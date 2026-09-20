#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "types.h"

class SmartMQTTClient {
public:
    SmartMQTTClient();
    void begin(QueueHandle_t cmdQueue);
    void update(); // Called from Core 0 network task
    void publishRelayState(uint8_t channel, bool state);
    void publishFullStatus(const SystemState_t& state);
    void publishHADiscovery();

    bool isConnected() { return mqttClient.connected(); }

private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    QueueHandle_t commandQueue;
    uint32_t lastReconnectAttempt;
    bool haDiscoverySent;

    void reconnect();
    void handleMessage(char* topic, byte* payload, unsigned int length);
    static void mqttCallbackWrapper(char* topic, byte* payload, unsigned int length);
};

extern SmartMQTTClient mqttManager;
