#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "types.h"

class SmartWebServer {
public:
    SmartWebServer();
    void begin(QueueHandle_t cmdQueue);
    void update(); // Called in Core 0 network loop
    void broadcastState(const SystemState_t& state);

private:
    WebServer server;
    WebSocketsServer wsServer;
    QueueHandle_t commandQueue;
    uint32_t lastBroadcastTime;

    void setupRoutes();
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
    static void webSocketEventWrapper(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
};

extern SmartWebServer webServerCtrl;
