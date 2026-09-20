# Hardware Specifications, Requirements & Schematics

## 1. System Requirements & Functional Specifications

### 1.1 Core Objectives
The **Advanced IoT Home Automation System** is an industrial-grade, edge-resilient smart controller designed for 4-channel appliance switching. The system maintains continuous, full offline operation through physical inputs and local line-of-sight wireless control, while offering high-speed network connectivity and remote management via a Raspberry Pi server.

### 1.2 Functional Requirements
| Subsystem | Requirement | Implementation Detail |
| :--- | :--- | :--- |
| **Power Switching** | Independent switching of 4 high-voltage AC loads (up to 250V AC / 10A per channel). | Optocoupled 4-channel relay matrix with isolated ground & flyback protection. |
| **Edge Autonomy** | Zero-latency local switching without reliance on Wi-Fi, routers, or servers. | Dedicated hardware interrupt / tight polling loop running on ESP32 Core 1. |
| **Physical Feedback** | Visual confirmation of active circuits and system state. | 4 status LEDs mapped 1:1 to relay channels, auto-dimmed via hardware PWM. |
| **Ambient Adaptation** | Non-intrusive nighttime illumination. | LDR sensor on ADC1 measuring ambient lux and adjusting LED brightness via EMA filter. |
| **Wireless Input** | Secondary local wireless line-of-sight control. | 38kHz IR receiver (VS1838B / TSOP38238) decoding NEC/RC5 protocols. |
| **Power-Cut Recovery** | Restore previous operational states after grid power failure. | Persistent state storage in ESP32 Non-Volatile Storage (NVS / Preferences API). |
| **System Diagnostics** | Hard reboot and network reconfiguration. | Dedicated external button (short press = reboot; hold 5s = Wi-Fi config AP mode). |
| **Network Interfaces** | Local browser UI, REST API, MQTT telemetry, and Raspberry Pi CLI. | Dual-core FreeRTOS task on Core 0 running HTTP server, WebSockets, and MQTT client. |

---

## 2. ESP32 Master Pin Allocation Table

> **Pin Selection Safeguards:**
> - Avoids boot-strapping pins (`GPIO 0, 2, 12, 15`) that cause boot failure or relay chatter.
> - LDR is positioned on **ADC1 (GPIO 34)** because ESP32's ADC2 is disabled when Wi-Fi is transmitting.
> - Status LEDs utilize hardware **LEDC PWM channels (0–3)** for flicker-free dimming.

| Pin | Direction | Component Function | Peripheral Mode | Hardware Electrical Notes |
| :--- | :--- | :--- | :--- | :--- |
| **GPIO 25** | Output | **Relay 1 Control** | Digital Out | Active LOW trigger to optocoupler input |
| **GPIO 26** | Output | **Relay 2 Control** | Digital Out | Active LOW trigger to optocoupler input |
| **GPIO 27** | Output | **Relay 3 Control** | Digital Out | Active LOW trigger to optocoupler input |
| **GPIO 14** | Output | **Relay 4 Control** | Digital Out | Active LOW trigger to optocoupler input |
| **GPIO 16** | Input | **Button 1 (Tactile)** | `INPUT_PULLUP` | Momentary push button to GND (25ms debounce) |
| **GPIO 17** | Input | **Button 2 (Tactile)** | `INPUT_PULLUP` | Momentary push button to GND (25ms debounce) |
| **GPIO 18** | Input | **Button 3 (Tactile)** | `INPUT_PULLUP` | Momentary push button to GND (25ms debounce) |
| **GPIO 23** | Input | **Button 4 (Tactile)** | `INPUT_PULLUP` | Momentary push button to GND (25ms debounce) |
| **GPIO 4** | Output | **Status LED 1** | LEDC PWM (Ch 0) | Driven via 220Ω resistor; mirrors Relay 1 |
| **GPIO 5** | Output | **Status LED 2** | LEDC PWM (Ch 1) | Driven via 220Ω resistor; mirrors Relay 2 |
| **GPIO 21** | Output | **Status LED 3** | LEDC PWM (Ch 2) | Driven via 220Ω resistor; mirrors Relay 3 |
| **GPIO 22** | Output | **Status LED 4** | LEDC PWM (Ch 3) | Driven via 220Ω resistor; mirrors Relay 4 |
| **GPIO 13** | Input | **IR Receiver (Data)** | Digital In / INT | 38kHz demodulated signal from VS1838B |
| **GPIO 34** | Input | **LDR Ambient Sensor** | ADC1_CH6 | Analog voltage divider (10kΩ pull-up to 3.3V) |
| **GPIO 32** | Input | **External Reset / Config**| `INPUT_PULLUP` | Momentary button to GND (Short = Reboot; 5s hold = AP Mode) |
| **GPIO 2**  | Output | **On-Board Blue LED** | Digital Out | Solid ON when Wi-Fi connected; OFF when disconnected/AP |

---

## 3. Electrical Schematics & Interface Circuits

### 3.1 4-Channel Relay Isolation Circuit
To protect the ESP32 from high-voltage spikes and electromagnetic interference (EMI):
```
ESP32 (3.3V Logic)               Optocoupled Relay Module (5V Isolated)
┌─────────────────┐             ┌─────────────────────────────────────┐
│                 │             │                                     │
│         VCC 3.3V├─────────────┤ VCC (Optocoupler Anodes)            │
│                 │             │                                     │
│   GPIO 25 (Ch1) ├─────────────┤ IN1 (Cathode through internal LED) │
│   GPIO 26 (Ch2) ├─────────────┤ IN2                                 │
│   GPIO 27 (Ch3) ├─────────────┤ IN3                                 │
│   GPIO 14 (Ch4) ├─────────────┤ IN4                                 │
│                 │             │                                     │
│                 │             │ [REMOVE JD-VCC JUMPER]              │
│                 │             │ JD-VCC ◄─── External 5V DC Supply   │
│                 │             │ GND    ◄─── External 5V DC GND      │
└─────────────────┘             └─────────────────────────────────────┘
```

> **Complete Galvanic Isolation:**
> Remove the blue `VCC-JDVCC` jumper on the relay board! Connect `VCC` to ESP32 3.3V, and connect `JD-VCC` and `GND` directly to your dedicated 5V power supply. This ensures relay coil switching noise does not enter the ESP32 ground plane.

### 3.2 Inductive Load Snubber Network
When driving inductive loads (fans, fluorescent ballasts, refrigerator compressors), the collapsing magnetic field produces a high-voltage spark across relay contacts.
* Install an **RC Snubber** across each relay's `NO` and `COM` terminals:
  * **Capacitor:** `100nF (0.1µF) 275V AC X2 safety-rated metallized film`
  * **Resistor:** `100Ω 1W or 2W flame-proof metal oxide`

### 3.3 LDR Ambient Light Voltage Divider
```
        +3.3V (ESP32)
          │
          │
         ┌┴┐
         │ │  10kΩ 1% Resistor
         │ │
         └┬┘
          ├───► To GPIO 34 (ADC1_CH6)
          │
         ┌┴┐
         │/│  LDR (Light Dependent Resistor, 5mm / 10k-20kΩ light resistance)
         └┬┘
          │
         GND
```
* **Behavior:**
  * **Bright room:** LDR resistance drops (~1kΩ) -> Voltage at GPIO 34 approaches 0V (ADC low).
  * **Dark room:** LDR resistance rises (>500kΩ) -> Voltage at GPIO 34 approaches 3.3V (ADC high).
  * *Firmware logic calculates Lux inversely and lowers LED PWM duty cycle at night.*

### 3.4 38kHz IR Receiver (VS1838B / TSOP38238)
```
          +3.3V (ESP32)
            │
           ┌┴┐ 100Ω Resistor
           └┬┘
            ├───► VCC Pin (Pin 3 of VS1838B)
            │
           ─── 4.7µF Electrolytic Filter Cap
           ───
            │
           GND ─► GND Pin (Pin 2 of VS1838B)
            
 ESP32 GPIO 13 ─► OUT Pin (Pin 1 of VS1838B) [With 10kΩ pull-up to 3.3V]
```

### 3.5 Status LEDs (PWM Driven)
```
 ESP32 GPIO (4, 5, 21, 22) ───[ 220Ω ]───►| (LED Anode) ───► GND (Cathode)
```
* Software controls brightness from 0% (off) up to dynamic max (10% to 100% depending on ambient darkness).

---

## 4. Power Architecture

* **Primary Supply:** 5V 2A regulated power supply (e.g., Hi-Link HLK-PM01 AC-DC module or 5V 2A industrial SMPS).
* The 5V rail powers:
  * Relay coils (`JD-VCC`).
  * ESP32 Board `5V / VIN` pin (fed into on-board AMS1117-3.3V regulator).
* High-voltage AC mains wiring (Live & Neutral) must maintain a minimum of **5mm creepage distance** from the low-voltage DC traces on the PCB.
