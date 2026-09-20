#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>

class WiFiManager {
public:
    WiFiManager();
    void begin();
    void update(); // Called from Core 0 network task
    
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    bool isAPMode() const { return apModeActive; }
    void startAP();
    void setCredentials(const String& ssid, const String& pass);
    String getIP() const;

private:
    bool apModeActive;
    uint32_t lastCheckTime;
    Preferences prefs;
    String ssid;
    String pass;

    void loadCredentials();
};

extern WiFiManager wifiMgr;
