# EasyEDA Circuit Diagram Implementation Guide & Engineering Notes

This document provides a **step-by-step methodology** to complete the remaining sections of the **Advanced IoT Home Automation System** schematic in EasyEDA Standard (or Pro), starting immediately after the AC-DC power supply section.

---

## 🗺️ Architectural Roadmap

Your schematic is logically divided into 7 distinct stages:

```
[ Stage 1: Power Supply ] (Completed: HLK-5M05 + C1 1000µF)
         │
         ├──► [ Stage 2: ESP32 DevKit Core Socket ]
         │          │
         │          ├──► [ Stage 3: 4-Channel Optocoupled Relay Interface ]
         │          ├──► [ Stage 4: 5x Tactile Push Buttons & Pull-ups ]
         │          ├──► [ Stage 5: 6x Status & Indicator LEDs ]
         │          ├──► [ Stage 6: Sensors (LDR Ambient + 15120P IR) ]
         │          └──► [ Stage 7: Expansion Headers (I2C, SPI, UART) ]
```

---

## ⚡ Golden Pre-Flight Rules for EasyEDA

1. **Use Net Labels / NetPorts Instead of Long Spaghetti Wires:**
   - In EasyEDA, press `P` or select **NetFlag** / **NetPort** from the Wiring Tools palette.
   - Use standard net names: `+5V`, `+3.3V`, `GND`.
   - Any wire labeled with `+5V` automatically connects across the entire schematic without needing long diagonal lines that clutter the drawing.
2. **Never Create 4-Way Cross Junctions:**
   - Always stagger wire intersections into **two 3-way "T" junctions**. In schematic CAD, 4-way cross lines often lose their connection dot during export or editing, creating phantom disconnects.
3. **Check Pin 1 Orientation on All Headers:**
   - Square pad = Pin 1. Always verify orientation before wiring.

---

## Step 1: ESP32 NodeMCU V1 DevKit Socket (30-Pin)

The central controller is a standard 30-pin ESP32-WROOM-32 DevKit V1 board. On the PCB, this is implemented as **two 1x15 2.54mm female pin headers**.

### 1.1 EasyEDA Part Selection
- **Search Term:** `ESP32-DEVKIT-V1` or `Header-Female-2.54_1x15` (x2)
- **Footprint:** `HDR-TH_15P-P2.54-V-F` (Spaced **22.86mm / 0.9"** or **25.4mm / 1.0"** center-to-center).
- **LCSC Part #:** `C22550` (1x15 2.54mm female header).

### 1.2 Socket Pinout & Connections

```
              LEFT HEADER (J1)                 RIGHT HEADER (J2)
              ┌───────────────┐               ┌───────────────┐
   EN (RST) ──┤ 1          15 ├── VIN (+5V)   ├── 16       30 ├── 3V3 (+3.3V)
  GPIO36/VP ──┤ 2          14 ├── GND         ├── 17       29 ├── GND
  GPIO39/VN ──┤ 3          13 ├── GPIO13 (IR) ├── 18       28 ├── GPIO23 (MOSI)
     GPIO34 ──┤ 4          12 ├── GPIO12 (PWR)├── 19       27 ├── GPIO22 (SCL)
     GPIO35 ──┤ 5          11 ├── GPIO14 (RL4)├── 20       26 ├── GPIO1 (TX0)
     GPIO32 ──┤ 6          10 ├── GPIO27 (RL3)├── 21       25 ├── GPIO3 (RX0)
     GPIO33 ──┤ 7           9 ├── GPIO26 (RL2)├── 22       24 ├── GPIO21 (SDA)
     GPIO25 ──┤ 8           8 ├── GPIO25 (RL1)├── 23       23 ├── GPIO19 (MISO)
              └───────────────┘               └───────────────┘
```

### 1.3 Power Wiring
- Connect **`VIN` (Pin 15)** to Net Label `+5V`.
- Connect **`GND` (Pin 14)** and **`GND` (Pin 29)** to Net Label `GND`.
- Connect **`3V3` (Pin 30)** to Net Label `+3.3V`.

> [!IMPORTANT]
> **PRECAUTION — Width of 30-Pin DevKit:**
> There are two common physical widths for 30-pin ESP32 DevKits on the market:
> - **Narrow:** 22.86mm (0.9 inch pitch between header rows) — most common NodeMCU V1.
> - **Wide:** 25.4mm (1.0 inch pitch between header rows).
> Measure your physical board with digital calipers before finalizing your PCB footprint spacing!

---

## Step 2: 4-Channel Relay Control Interface

Controls 4 appliance relays with complete galvanic isolation between the noisy 5V coil rail and the sensitive 3.3V ESP32 logic.

### 2.1 EasyEDA Part Selection
- **Signal Header (J3):** 1x6 Pin Header 2.54mm (`HDR-TH_6P-P2.54-V-M` or female).
- **Power/Isolation Header (J4):** 1x3 Pin Header 2.54mm (`HDR-TH_3P-P2.54-V-M`).

### 2.2 Schematic Wiring

```
   ESP32 DevKit                          Relay Module Interface
  ┌────────────┐                         ┌──────────────────────┐
  │    GPIO 25 ├─────────────────────────┤ IN1                  │
  │    GPIO 26 ├─────────────────────────┤ IN2                  │
  │    GPIO 27 ├─────────────────────────┤ IN3                  │
  │    GPIO 14 ├─────────────────────────┤ IN4                  │
  │            │                         │                      │
  │       3V3  ├─────────────────────────┤ VCC (Opto Anodes)    │
  │            │                         │                      │
  │  Net: +5V  ├─────────────────────────┤ JD-VCC (Coil Power)  │
  │  Net: GND  ├─────────────────────────┤ GND (Coil Return)    │
  └────────────┘                         └──────────────────────┘
                                         ⚠️ BLUE JUMPER REMOVED!
```

### 2.3 Wiring Steps
1. Place a **1x6 connector** (Pins: `GND`, `IN1`, `IN2`, `IN3`, `IN4`, `VCC`).
   - Pin 1 (`GND`): Connect to Net `GND`.
   - Pin 2 (`IN1`): Connect to `GPIO 25`.
   - Pin 3 (`IN2`): Connect to `GPIO 26`.
   - Pin 4 (`IN3`): Connect to `GPIO 27`.
   - Pin 5 (`IN4`): Connect to `GPIO 14`.
   - Pin 6 (`VCC`): Connect to Net `+3.3V` *(Powers optocoupler internal LEDs)*.
2. Place a **1x3 connector** for the JD-VCC selection block (Pins: `VCC`, `JD-VCC`, `GND`).
   - Pin 1 (`VCC`): Leave NC (open) or tie to 3.3V.
   - Pin 2 (`JD-VCC`): Connect directly to Net `+5V`.
   - Pin 3 (`GND`): Connect directly to Net `GND`.

> [!WARNING]
> **CRITICAL PRECAUTION — The Blue Shunt Jumper:**
> Relay boards ship with a blue jumper linking `VCC` and `JD-VCC`. **PULL THIS JUMPER OFF.**
> If left on, 5V relay coil kickback and flyback noise dump straight into the ESP32's 3.3V logic line, causing immediate Wi-Fi crashes and phantom resets.

---

## Step 3: Physical Tactile Push Buttons & Pull-ups

The system features 5 momentary tactile push buttons:
- **Buttons 1–4:** Manual appliance override (Relays 1–4).
- **Button 5 (Reset / Config):** Short press = hard reboot; 5-second hold = AP configuration mode.

### 3.1 EasyEDA Part Selection
- **Switches (SW1–SW5):** 6x6x5mm 4-Pin Through-Hole Tactile Switch.
  - Footprint: `SW-TH_4P-L6.0-W6.0-P4.50-LS6.50` (LCSC: `C318884`).
- **Resistors (R1–R3):** 10kΩ 1/4W Through-Hole Axial (`R-TH_AXIAL-0.4` or 0805 SMD, LCSC: `C25804`).

### 3.2 Schematic Wiring Diagram

```
         +3.3V               +3.3V               +3.3V
           │                   │                   │
         [10k] R1            [10k] R2            [10k] R3
           │                   │                   │
           ├────── GPIO 35     ├────── GPIO 36     ├────── GPIO 39
           │ (Button 1)        │ (Button 2)        │ (Button 3)
         [SW1]               [SW2]               [SW3]
           │                   │                   │
          GND                 GND                 GND

    (GPIO 16 and GPIO 17 use ESP32 internal INPUT_PULLUP — no external resistors needed)
           │                   │
           ├────── GPIO 16     ├────── GPIO 17
           │ (Button 4)        │ (Reset / AP Mode)
         [SW4]               [SW5]
           │                   │
          GND                 GND
```

### 3.3 Wiring Steps
1. **Buttons 1, 2, 3 (GPIO 35, 36, 39):**
   - Connect one side of each switch to `GND`.
   - Connect the other side to the respective GPIO pin.
   - Connect a **10kΩ resistor** between that GPIO line and `+3.3V`.
2. **Button 4 (GPIO 16) & Config Button (GPIO 17):**
   - Connect one side to `GND`.
   - Connect the other side directly to `GPIO 16` and `GPIO 17`.

> [!CAUTION]
> **CRITICAL PRECAUTION — Input-Only GPI Pins (GPIO 34, 35, 36, 39):**
> GPIO 34, 35, 36 (VP), and 39 (VN) are **Input-Only** pins on the ESP32 silicon. They **DO NOT HAVE INTERNAL PULL-UP RESISTORS**.
> If you omit the external 10kΩ resistors on Buttons 1, 2, and 3, those pins will float, causing the relays to switch erratically like machine guns!

---

## Step 4: Status & Dedicated Indicator LEDs (6-Channel LEDC)

The system provides 6 auto-dimmed status LEDs:
- **LED 1–4:** Relay 1–4 state indicators.
- **LED 5 (Power):** Dedicated system power indicator.
- **LED 6 (Wi-Fi):** Network connectivity & MQTT telemetry indicator.

### 4.1 EasyEDA Part Selection
- **LEDs (LED1–LED6):** 3mm or 5mm Through-Hole LEDs (`LED-TH_D3.0mm` or `LED-TH_D5.0mm`).
- **Resistors (R4–R8):** 220Ω 1/4W Through-Hole Axial (`R-TH_AXIAL-0.4`, LCSC: `C17561`).
- **Resistor (R9 - Power LED):** 330Ω 1/4W Through-Hole Axial (LCSC: `C17628`).

### 4.2 Schematic Wiring Diagram

```
   ESP32 GPIO               Resistor           LED           Ground
   ────────────────────────────────────────────────────────────────
   GPIO 4  (Status 1) ────► [ 220Ω ] ────►|─ (Green) ────► GND
   GPIO 15 (Status 2) ────► [ 220Ω ] ────►|─ (Green) ────► GND  ⚡ MTDO Strapping Safe
   GPIO 32 (Status 3) ────► [ 220Ω ] ────►|─ (Green) ────► GND
   GPIO 33 (Status 4) ────► [ 220Ω ] ────►|─ (Green) ────► GND
   GPIO 12 (Power LED)────► [ 330Ω ] ────►|─ (Red)   ────► GND  ⚡ MTDI Hardware Pull-Down
   GPIO 2  (Wi-Fi LED)────► [ 220Ω ] ────►|─ (Blue)  ────► GND  ⚡ Boot Strapping Safe
```

### 4.3 Wiring Steps
1. Wire each GPIO pin to the **Anode (+, longer leg)** of the LED via its series resistor.
2. Connect all LED **Cathodes (–, flat edge)** directly to Net `GND`.

> [!IMPORTANT]
> **CRITICAL PRECAUTION — GPIO 12 (MTDI) Boot-Strapping Trap:**
> `GPIO 12` is a high-risk strapping pin. If driven HIGH at boot, the ESP32 sets the flash memory voltage to 1.8V instead of 3.3V, causing a permanent boot loop!
> Wiring the Power LED as **Active-HIGH** (GPIO 12 ──► 330Ω ──► LED ──► GND) pulls GPIO 12 solidly to `GND` through the 330Ω resistor at startup, **guaranteeing 100% reliable 3.3V flash boot**!

---

## Step 5: Ambient Light Sensor (LDR Voltage Divider)

Measures ambient room lux to auto-dim all 6 LEDs at night.

### 5.1 EasyEDA Part Selection
- **LDR (R_LDR):** 5mm Photoresistor GL5528 (`R-TH_GL5528`, LCSC: `C74015`).
- **Divider Resistor (R10):** 10kΩ 1% Metal Film (`R-TH_AXIAL-0.4`, LCSC: `C25804`).

### 5.2 Schematic Wiring Diagram

```
         +3.3V
           │
         ┌─┴─┐
         │LDR│ (Photoresistor: ~500Ω in daylight, ~50kΩ in dark)
         └─┬─┘
           ├───► GPIO 34 (ADC1 Channel 6)
         ┌─┴─┐
         │10k│ R10 (1% Divider Resistor)
         └─┬─┘
           │
          GND
```

### 5.3 Wiring Steps
1. Connect top terminal of LDR to Net `+3.3V`.
2. Connect bottom terminal of LDR to Net `GPIO 34`.
3. Connect a 10kΩ resistor from `GPIO 34` down to Net `GND`.

> [!WARNING]
> **CRITICAL PRECAUTION — ADC1 vs ADC2 Wi-Fi Conflict:**
> The ESP32 has two ADCs. **ADC2 (GPIO 0, 2, 4, 12-15, 25-27) is physically disabled by the silicon whenever the Wi-Fi driver is active.**
> `GPIO 34` resides on **ADC1 (Channel 6)**, allowing uninterrupted analog light sensing while serving WebSockets and MQTT packets simultaneously. Never move the LDR to an ADC2 pin!

---

## Step 6: 15120P 38kHz Infrared Receiver

Provides secondary local line-of-sight wireless control up to 15 meters using standard NEC / RC5 remote controls.

### 6.1 EasyEDA Part Selection
- **IR Receiver (U3):** 15120P / TSOP38238 / VS1838B (SIP-3 Through-Hole).
  - Footprint: `HDR-TH_1x3-P2.54` (LCSC: `C520970` or `C15187`).
- **Decoupling Cap (C2):** 100nF Ceramic 50V (`CAP-TH_D5.0-P2.50` or 0805 SMD, LCSC: `C49678`).

### 6.2 Pinout & Schematic Wiring

```
         15120P Pinout
         (Front View with Lens Bulge Facing You)
         ┌───────────────┐
         │     ( ● )     │
         │    15120P     │
         └──┬────┬────┬──┘
            1    2    3
           OUT  GND  VCC
            │    │    │
            │    │    └──► Net: +3.3V ──┐
            │    │                      ┴  C2 (100nF)
            │    └───────► Net: GND  ───┬──┘
            │                           │
            └────────────► Net: GPIO 13
```

### 6.3 Wiring Steps
1. Connect **Pin 1 (OUT)** directly to `GPIO 13`.
2. Connect **Pin 2 (GND)** directly to Net `GND`.
3. Connect **Pin 3 (VCC)** directly to Net `+3.3V`.
4. Place a **100nF ceramic capacitor** directly across Pin 3 (+3.3V) and Pin 2 (GND) right next to the sensor footprint.

> [!CAUTION]
> **CRITICAL PRECAUTION — IR Sensor Pinout Trap:**
> The **15120P** pinout is:
> - **Pin 1 = OUT** | **Pin 2 = GND** | **Pin 3 = VCC**
> Beware of cheap clone sensors (like some VS1838 variants) that swap Pin 2 and Pin 3 (Pin 2=VCC, Pin 3=GND). Reversing VCC and GND will instantly destroy the sensitive internal bipolar photodiode pre-amplifier!

---

## Step 7: Reserved Expansion & Diagnostic Headers

To make your PCB future-proof, break out the unallocated hardware communication busses to standard 2.54mm male pin headers.

### 7.1 Expansion Headers Pinout Table

| Header | Bus Type | Pin 1 | Pin 2 | Pin 3 | Pin 4 | Pin 5 | Pin 6 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **J5 (1x4)** | **I2C** (OLED / BME280) | `+3.3V` | `GND` | `GPIO 22` (SCL) | `GPIO 21` (SDA) | — | — |
| **J6 (1x6)** | **VSPI** (SD Card / TFT) | `+3.3V` | `GND` | `GPIO 18` (SCK) | `GPIO 19` (MISO)| `GPIO 23` (MOSI)| `GPIO 5` (CS) |
| **J7 (1x3)** | **UART0** (Serial Debug) | `GND`   | `GPIO 1` (TX0) | `GPIO 3` (RX0) | — | — | — |

---

## 📋 Comprehensive Bill of Materials (BOM) Table

All components utilize standard through-hole footprints available in the **EasyEDA System Library**:

| Designator | Component Description | Value / Rating | EasyEDA Footprint | LCSC Part # |
| :--- | :--- | :--- | :--- | :--- |
| **PS1** | Hi-Link Isolated AC-DC SMPS | 5V 1A (5W) | `TH_HLK-5M05` | `C209907` |
| **C1** | Radial Aluminum Electrolytic | 1000µF 16V (Low ESR) | `CAP-D10.0×F5.0` | `C12328` |
| **C2** | Ceramic MLCC Decoupling | 100nF 50V X7R | `CAP-TH_D5.0-P2.50` | `C49678` |
| **U1** | ESP32 NodeMCU V1 Socket | 30-Pin (2x 1x15 Headers) | `HDR-TH_15P-P2.54-V-F` | `C22550` |
| **J3** | Relay Logic Control Header | 1x6 Pin Male Header | `HDR-TH_6P-P2.54-V-M` | `C37372` |
| **J4** | Relay JD-VCC Isolation Header | 1x3 Pin Male Header | `HDR-TH_3P-P2.54-V-M` | `C37374` |
| **SW1–SW5** | Momentary Pushbuttons | 6x6x5mm 4-Pin Tactile | `SW-TH_4P-L6.0-W6.0` | `C318884` |
| **R1–R3, R10**| Metal Film Resistors (Pull-ups/LDR) | 10kΩ 1/4W 1% | `R-TH_AXIAL-0.4` | `C25804` |
| **R4–R8** | LED Current Limiting Resistors | 220Ω 1/4W 5% | `R-TH_AXIAL-0.4` | `C17561` |
| **R9** | Power LED Resistor (Boot Pull-down)| 330Ω 1/4W 5% | `R-TH_AXIAL-0.4` | `C17628` |
| **LED1–LED4**| Relay State Mirror LEDs | 3mm Green / Blue | `LED-TH_D3.0mm` | `C965799` |
| **LED5** | Dedicated Power LED | 3mm Red | `LED-TH_D3.0mm` | `C965798` |
| **LED6** | Dedicated Wi-Fi Status LED | 3mm Blue | `LED-TH_D3.0mm` | `C965800` |
| **R_LDR** | Ambient Light Photoresistor | 5mm GL5528 | `R-TH_GL5528` | `C74015` |
| **U3** | 38kHz Optical IR Receiver | 15120P (15m, 180° FOV)| `HDR-TH_1x3-P2.54` | `C520970` |
| **J5–J7** | Expansion Busses (I2C, SPI, UART)| Headers 1x4, 1x6, 1x3 | `HDR-TH_P2.54-V-M` | `C37372` |

---

## ✅ Pre-PCB Electrical Rule Check (ERC) Checklist

Before clicking **Convert Schematic to PCB** in EasyEDA, run through this final checklist:

- [ ] **Run Design Manager ERC:** Click `Design -> Check Netlist` or `Design Manager` in the left panel. Verify **0 unconnected pins** (excluding intentionally unused pins marked with a red cross `No Connect Flag`).
- [ ] **Verify Net Labels:** Ensure spelling matches exactly (`+5V` is not typed as `5V`, `GND` is not typed as `Ground`).
- [ ] **Capacitor Polarity Check:** Confirm C1 Pin 1 is tied to `+5V` and Pin 2 is tied to `GND`.
- [ ] **Input-Only Resistors:** Confirm GPIO 35, 36, and 39 have their 10kΩ pull-up resistors connected to `+3.3V`.
- [ ] **LDR ADC Channel:** Confirm the LDR divider enters `GPIO 34` (ADC1).
- [ ] **Relay VCC:** Confirm the relay control header pin labeled `VCC` is tied to `+3.3V`, and `JD-VCC` is tied to `+5V`.
