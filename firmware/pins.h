#pragma once

// =============================================================================
// Advanced IoT Home Automation System - Hardware Pin Definitions
// =============================================================================

// 4-Channel Relays (Active LOW for standard optocoupled relay modules)
#define PIN_RELAY_1         25
#define PIN_RELAY_2         26
#define PIN_RELAY_3         27
#define PIN_RELAY_4         14

#define RELAY_ACTIVE_LEVEL  LOW
#define RELAY_INACTIVE_LEVEL HIGH

// 4 Physical Tactile Override Buttons (Active LOW with internal pull-up)
#define PIN_BUTTON_1        16
#define PIN_BUTTON_2        17
#define PIN_BUTTON_3        18
#define PIN_BUTTON_4        23

// 4 Status Feedback LEDs (Driven by ESP32 LEDC PWM)
#define PIN_LED_1           4
#define PIN_LED_2           5
#define PIN_LED_3           21
#define PIN_LED_4           22

// LEDC PWM Channel Allocations (0-3)
#define LEDC_CH_LED1        0
#define LEDC_CH_LED2        1
#define LEDC_CH_LED3        2
#define LEDC_CH_LED4        3
#define LEDC_PWM_FREQ       5000  // 5 kHz (smooth, zero audible coil whine)
#define LEDC_PWM_RES        8     // 8-bit resolution (0 - 255)

// Infrared Receiver (VS1838B / TSOP38238, 38kHz demodulated)
#define PIN_IR_RECV         13

// Ambient Light Sensor (LDR on ADC1 Channel 6)
// NOTE: Must be on ADC1 (GPIO 32-39) to operate concurrently with Wi-Fi!
#define PIN_LDR_ADC         34

// External Reset / Mode Button (GPIO 32 supports internal pull-up)
#define PIN_CONFIG_RESET    32

// On-Board Blue Status LED (Active HIGH on ESP32 DevKit WROOM)
#define PIN_ONBOARD_LED     2

