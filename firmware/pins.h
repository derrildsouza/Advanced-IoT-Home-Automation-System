#pragma once

// =============================================================================
// Advanced IoT Home Automation System - Master Hardware Pin Definitions
// 30-Pin ESP32-WROOM-32 Dev Board (100% GPIO Utilization - 25/25 Pins Allocated)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 4-Channel Appliance Relays
// -----------------------------------------------------------------------------
// Clean digital GPIOs with no boot-strapping conflicts or power-on PWM hash
#define PIN_RELAY_1         25    // Relay 1 Control (Left Pin 8)
#define PIN_RELAY_2         26    // Relay 2 Control (Left Pin 9)
#define PIN_RELAY_3         27    // Relay 3 Control (Left Pin 10)
#define PIN_RELAY_4         14    // Relay 4 Control (Left Pin 11)

#define RELAY_ACTIVE_LEVEL  LOW   // Optocoupled active-LOW trigger
#define RELAY_INACTIVE_LEVEL HIGH

// -----------------------------------------------------------------------------
// 2. Physical Tactile Override Buttons (Active LOW)
// -----------------------------------------------------------------------------
// NOTE: GPIO 35, 36 (VP), 39 (VN) are Input-Only (GPI) without internal pull-ups.
// They MUST be fitted with external 10kΩ pull-up resistors tied to 3.3V!
// GPIO 16 (RX2) and GPIO 17 (TX2) have internal pull-ups enabled in firmware.
#define PIN_BUTTON_1        35    // Button 1 (Left Pin 5)  - Requires ext 10kΩ pull-up
#define PIN_BUTTON_2        36    // Button 2 (Left Pin 2 / VP) - Requires ext 10kΩ pull-up
#define PIN_BUTTON_3        39    // Button 3 (Left Pin 3 / VN) - Requires ext 10kΩ pull-up
#define PIN_BUTTON_4        16    // Button 4 (Right Pin 10 / RX2) - Internal INPUT_PULLUP

// External Reset / Mode Button (Short press = Reboot, Hold 5s = AP Mode)
#define PIN_CONFIG_RESET    17    // Reset / Config AP (Right Pin 9 / TX2) - Internal INPUT_PULLUP

// -----------------------------------------------------------------------------
// 3. Status & Dedicated Indicator LEDs (6-Channel Hardware LEDC PWM)
// -----------------------------------------------------------------------------
// 4 Relay State Mirror LEDs
#define PIN_LED_1           4     // Status LED 1 (Right Pin 11)
#define PIN_LED_2           15    // Status LED 2 (Right Pin 13 / MTDO strapping safe)
#define PIN_LED_3           32    // Status LED 3 (Left Pin 6)
#define PIN_LED_4           33    // Status LED 4 (Left Pin 7)

// Dedicated System Indicator LEDs
// GPIO 12: Active-HIGH circuit (Anode to GP12, Cathode to GND via 330Ω) acts as a hardware
// pull-down during boot, GUARANTEEING 3.3V flash boot voltage (MTDI strapping safety).
#define PIN_POWER_LED       12    // Power Indicator LED (Left Pin 12 - Always ON, Auto-Dimmed)
#define PIN_WIFI_LED        2     // Wi-Fi Status LED (Right Pin 12 - Onboard Blue LED, Auto-Dimmed)
#define PIN_ONBOARD_LED     PIN_WIFI_LED

// LEDC Hardware PWM Channel Allocations (Channels 0–5)
#define LEDC_CH_LED1        0     // Relay 1 Status Mirror
#define LEDC_CH_LED2        1     // Relay 2 Status Mirror
#define LEDC_CH_LED3        2     // Relay 3 Status Mirror
#define LEDC_CH_LED4        3     // Relay 4 Status Mirror
#define LEDC_CH_PWR         4     // Power Indicator (Auto-Dimmed by LDR)
#define LEDC_CH_WIFI        5     // Wi-Fi Status (Auto-Dimmed by LDR)

#define LEDC_PWM_FREQ       5000  // 5 kHz (smooth, zero audible coil whine)
#define LEDC_PWM_RES        8     // 8-bit resolution (0 - 255 duty cycle)

// -----------------------------------------------------------------------------
// 4. Sensors
// -----------------------------------------------------------------------------
// Infrared Receiver (TSOP38238 / 15120P, 38kHz demodulated Active-LOW stream)
#define PIN_IR_RECV         13    // IR Data Input (Left Pin 13)

// Ambient Light Sensor (LDR Voltage Divider on ADC1 Channel 6)
// NOTE: Must be on ADC1 (GPIO 32-39) to operate concurrently with Wi-Fi!
#define PIN_LDR_ADC         34    // LDR Analog In (Left Pin 4)

// -----------------------------------------------------------------------------
// 5. Reserved Hardware Communication Busses
// -----------------------------------------------------------------------------
// Reserved I2C Bus (Native Hardware Wire: OLED Displays, RTC, BME280 Sensors)
#define PIN_I2C_SDA         21    // I2C Serial Data (Right Pin 5)
#define PIN_I2C_SCL         22    // I2C Serial Clock (Right Pin 2)

// Reserved SPI Bus (Native Hardware VSPI: Micro-SD Storage, TFT Displays)
#define PIN_SPI_SCK         18    // VSPI Serial Clock (Right Pin 7)
#define PIN_SPI_MISO        19    // VSPI Master-In Slave-Out (Right Pin 6)
#define PIN_SPI_MOSI        23    // VSPI Master-Out Slave-In (Right Pin 1)
#define PIN_SPI_CS          5     // VSPI Chip Select (Right Pin 8)

// Reserved UART0 Bus (Hardware Serial / CP2102 USB Bridge)
#define PIN_UART0_TX        1     // UART0 TX (Right Pin 3)
#define PIN_UART0_RX        3     // UART0 RX (Right Pin 4)

