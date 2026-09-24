#include "wifi_manager.h"
#include "config.h"
#include "pins.h"

WiFiManager wifiMgr;

WiFiManager::WiFiManager() :
    apModeActive(false),
    lastCheckTime(0)
{
}

void WiFiManager::loadCredentials() {
    prefs.begin("wifi_cfg", false);
    ssid = prefs.getString("ssid", DEFAULT_WIFI_SSID);
    pass = prefs.getString("pass", DEFAULT_WIFI_PASS);
}

void WiFiManager::setCredentials(const String& newSsid, const String& newPass) {
    ssid = newSsid;
    pass = newPass;
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
}

void WiFiManager::begin() {
    loadCredentials();

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);

    if (ssid.length() > 0 && ssid != "YOUR_WIFI_SSID") {
        Serial.printf("[WiFi] Connecting to %s...\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());
    } else {
        Serial.println("[WiFi] No valid credentials found. Starting AP mode.");
        startAP();
        return;
    }

    // Wait up to 10 seconds for initial connection
    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startMs < 10000)) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        if (MDNS.begin(HOSTNAME)) {
            Serial.printf("[mDNS] Responder started at http://%s.local\n", HOSTNAME);
        }
    } else {
        Serial.println("[WiFi] Connection failed. Fallback to AP Mode.");
        startAP();
    }
}

void WiFiManager::startAP() {
    apModeActive = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.printf("[WiFi AP] Started AP: %s (Pass: %s)\n", AP_SSID, AP_PASS);
    Serial.printf("[WiFi AP] IP Address: %s\n", WiFi.softAPIP().toString().c_str());
}

void WiFiManager::update() {
    if (apModeActive) {
        return;
    }

    uint32_t now = millis();
    if (now - lastCheckTime >= 10000) { // Check connection every 10 seconds
        lastCheckTime = now;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WiFi] Connection lost. Attempting reconnect...");
            WiFi.reconnect();
        }
    }
}

String WiFiManager::getIP() const {
    if (apModeActive) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}
