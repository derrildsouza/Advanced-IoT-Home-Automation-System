#include "web_server.h"
#include "web_dashboard.h"
#include "relay_controller.h"
#include "ambient_dimmer.h"
#include "ir_controller.h"
#include "wifi_manager.h"
#include <ArduinoJson.h>

SmartWebServer webServerCtrl;

SmartWebServer::SmartWebServer() :
    server(80),
    wsServer(81),
    commandQueue(nullptr),
    lastBroadcastTime(0)
{
}

void SmartWebServer::webSocketEventWrapper(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    webServerCtrl.handleWebSocketEvent(num, type, payload, length);
}

void SmartWebServer::begin(QueueHandle_t cmdQueue) {
    commandQueue = cmdQueue;
    setupRoutes();
    server.begin();
    wsServer.begin();
    wsServer.onEvent(webSocketEventWrapper);
    Serial.println("[HTTP] Web server started on port 80");
    Serial.println("[WS] WebSocket server started on port 81");
}

void SmartWebServer::setupRoutes() {
    // 1. Root dashboard UI
    server.on("/", HTTP_GET, [this]() {
        server.send(200, "text/html", INDEX_HTML);
    });

    // 2. REST API: Status JSON
    server.on("/api/status", HTTP_GET, [this]() {
        JsonDocument doc;
        JsonArray arr = doc["relays"].to<JsonArray>();
        for (int i = 0; i < 4; i++) {
            arr.add(relayCtrl.getState(i));
        }
        doc["ldr"] = ambientDimmer.getRawLdr();
        doc["led_brightness"] = ambientDimmer.getCalculatedBrightness();
        doc["uptime"] = millis() / 1000;

        char hexBuf[16];
        snprintf(hexBuf, sizeof(hexBuf), "0x%08X", irCtrl.getLastCode());
        doc["ir_code"] = hexBuf;

        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });

    // 3. REST API: Toggle Relay /api/relay/{0..3}/toggle
    for (uint8_t ch = 0; ch < 4; ch++) {
        String path = "/api/relay/" + String(ch) + "/toggle";
        server.on(path.c_str(), HTTP_POST, [this, ch]() {
            if (commandQueue) {
                RelayCommand_t cmd = { ch, -1, CommandSource::REST_API };
                xQueueSend(commandQueue, &cmd, 0);
            }
            server.send(200, "application/json", "{\"success\":true}");
        });

        // Set Relay state /api/relay/{ch} (e.g., ?state=on or ?state=off)
        String setPath = "/api/relay/" + String(ch);
        server.on(setPath.c_str(), HTTP_POST, [this, ch]() {
            int8_t target = -1;
            if (server.hasArg("state")) {
                String s = server.arg("state");
                s.toLowerCase();
                if (s == "on" || s == "1" || s == "true") target = 1;
                else if (s == "off" || s == "0" || s == "false") target = 0;
            }
            if (target >= 0 && commandQueue) {
                RelayCommand_t cmd = { ch, target, CommandSource::REST_API };
                xQueueSend(commandQueue, &cmd, 0);
            }
            server.send(200, "application/json", "{\"success\":true}");
        });
    }

    // 4. REST API: Bulk control /api/relays
    server.on("/api/relays", HTTP_POST, [this]() {
        if (server.hasArg("plain")) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, server.arg("plain"));
            if (!err && doc.containsKey("state")) {
                bool state = doc["state"].as<bool>();
                for (uint8_t i = 0; i < 4; i++) {
                    RelayCommand_t cmd = { i, (int8_t)(state ? 1 : 0), CommandSource::REST_API };
                    xQueueSend(commandQueue, &cmd, 0);
                }
            }
        }
        server.send(200, "application/json", "{\"success\":true}");
    });

    // 5. REST API: Wi-Fi setup
    server.on("/api/wifi", HTTP_POST, [this]() {
        if (server.hasArg("ssid") && server.hasArg("pass")) {
            wifiMgr.setCredentials(server.arg("ssid"), server.arg("pass"));
            server.send(200, "application/json", "{\"status\":\"saved, rebooting\"}");
            delay(1000);
            ESP.restart();
        } else {
            server.send(400, "application/json", "{\"error\":\"Missing ssid or pass\"}");
        }
    });

    // 6. REST API: Reboot
    server.on("/api/reboot", HTTP_POST, [this]() {
        server.send(200, "application/json", "{\"status\":\"rebooting\"}");
        delay(500);
        ESP.restart();
    });

    // 404 handler
    server.onNotFound([this]() {
        server.send(404, "text/plain", "Not Found");
    });
}

void SmartWebServer::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT && length > 0) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload, length);
        if (!err && doc.containsKey("cmd")) {
            String cmd = doc["cmd"].as<String>();
            if (cmd == "toggle" && doc.containsKey("ch")) {
                uint8_t ch = doc["ch"].as<uint8_t>();
                if (ch < 4 && commandQueue) {
                    RelayCommand_t rCmd = { ch, -1, CommandSource::WEB_UI };
                    xQueueSend(commandQueue, &rCmd, 0);
                }
            } else if (cmd == "all" && doc.containsKey("state")) {
                bool state = doc["state"].as<bool>();
                for (uint8_t i = 0; i < 4; i++) {
                    RelayCommand_t rCmd = { i, (int8_t)(state ? 1 : 0), CommandSource::WEB_UI };
                    xQueueSend(commandQueue, &rCmd, 0);
                }
            }
        }
    }
}

void SmartWebServer::broadcastState(const SystemState_t& state) {
    JsonDocument doc;
    JsonArray arr = doc["relays"].to<JsonArray>();
    for (int i = 0; i < 4; i++) {
        arr.add(state.relayStates[i]);
    }
    doc["ldr"] = state.ldrRaw;
    doc["led_brightness"] = state.ledBrightness;
    doc["uptime"] = state.uptimeSeconds;

    char hexBuf[16];
    snprintf(hexBuf, sizeof(hexBuf), "0x%08X", state.lastIRCode);
    doc["ir_code"] = hexBuf;

    String json;
    serializeJson(doc, json);
    wsServer.broadcastTXT(json);
}

void SmartWebServer::update() {
    server.handleClient();
    wsServer.loop();
}
