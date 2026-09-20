#pragma once

// =============================================================================
// Advanced IoT Home Automation System - System Configuration
// =============================================================================

// Device Identity
#define DEVICE_NAME             "SmartSwitch4Ch"
#define HOSTNAME                "smartswitch"
#define FIRMWARE_VERSION        "1.0.0"

// Load credentials from secrets.h if available
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #warning "secrets.h not found — using default placeholder credentials. Copy secrets.h.example to secrets.h!"
  #define WIFI_SSID       "YOUR_WIFI_SSID"
  #define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
  #define MQTT_BROKER_IP  "192.168.1.250"
  #define MQTT_PORT       1883
  #define MQTT_USER       ""
  #define MQTT_PASS       ""
#endif

// Wi-Fi Configuration
#define DEFAULT_WIFI_SSID       WIFI_SSID
#define DEFAULT_WIFI_PASS       WIFI_PASSWORD
#define AP_SSID                 "SmartSwitch-Setup"
#define AP_PASS                 "12345678"

// MQTT Configuration (for Raspberry Pi Broker)
#define DEFAULT_MQTT_BROKER     MQTT_BROKER_IP
#define DEFAULT_MQTT_PORT       MQTT_PORT
#define DEFAULT_MQTT_USER       MQTT_USER
#define DEFAULT_MQTT_PASS       MQTT_PASS
#define MQTT_CLIENT_ID          "ESP32_SmartSwitch_01"

// MQTT Topics
#define MQTT_TOPIC_PREFIX       "home/switch/"
#define MQTT_TOPIC_STATUS       "home/switch/status"
#define MQTT_TOPIC_COMMAND      "home/switch/cmd"

// Home Assistant MQTT Auto-Discovery Prefix
#define HA_DISCOVERY_PREFIX     "homeassistant"

// FreeRTOS Task & Queue Configuration
#define QUEUE_SIZE_COMMANDS     16
#define QUEUE_SIZE_EVENTS       16
#define TASK_STACK_HARDWARE     4096
#define TASK_STACK_NETWORK      8192

// Hardware Timing & Tuning
#define BUTTON_DEBOUNCE_MS      35
#define LDR_SAMPLE_INTERVAL_MS  250
#define LDR_EMA_ALPHA           0.15f   // Exponential moving average smoothing factor
#define MIN_LED_BRIGHTNESS      15      // Dim level at total darkness (night mode)
#define MAX_LED_BRIGHTNESS      255     // Bright level in full daylight

// Default NEC IR Codes (Can be learned/overridden in Web UI)
#define DEFAULT_IR_KEY_CH1      0xFFA25D   // Key '1' on standard mini remote
#define DEFAULT_IR_KEY_CH2      0xFF629D   // Key '2'
#define DEFAULT_IR_KEY_CH3      0xFFE21D   // Key '3'
#define DEFAULT_IR_KEY_CH4      0xFF22DD   // Key '4'
#define DEFAULT_IR_KEY_ALL_OFF  0xFF02FD   // Key 'Power' / 'OK'
