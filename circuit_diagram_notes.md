# EasyEDA Circuit Diagram Implementation Guide & Engineering Notes

This document provides a **step-by-step methodology** to complete the remaining sections of the **Advanced IoT Home Automation System** schematic in EasyEDA Standard (or Pro), customized to match your exact **`ESP32-DEVKIT-V1-30PIN1LAB`** schematic symbol pinout (`D25`, `D4`, `RX2`, `TX2`, `VN`, `VP`, etc.).

---

## 🗺️ Architectural Roadmap

Your schematic is logically divided into 7 distinct stages:

```
[ Stage 1: Power Supply ] (Completed: HLK-5M05 + C1 1000µF on VIN & GND)
         │
         ├──► [ Step 1: ESP32 DevKit Core Socket (ESP32-DEVKIT-V1-30PIN1LAB) ]
         │          │
         │          ├──► [ Step 2: 4-Channel Optocoupled Relay Interface ]
         │          ├──► [ Step 3: 5x Tactile Push Buttons & Pull-ups ]
         │          ├──► [ Step 4: 6x Status & Indicator LEDs ]
         │          ├──► [ Step 5: Sensors (LDR Ambient + 15120P IR) ]
         │          └──► [ Step 6: Expansion Headers (I2C, SPI, UART) ]
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

## Step 1: ESP32 NodeMCU V1 DevKit Symbol (`ESP32-DEVKIT-V1-30PIN1LAB`)

Here is your exact symbol layout from EasyEDA, showing the Left and Right pin columns along with their assigned IoT system functions:

```
                  U1: ESP32-DEVKIT-V1-30PIN1LAB
                  ┌───────────────────────────────┐
    Net: +3.3V ───┤ 3V3                       VIN ├─── Net: +5V (From HLK / C1 +)
      Net: GND ───┤ GND                      GND2 ├─── Net: GND (From HLK / C1 -)
 Status LED 2  ───┤ D15                       D13 ├─── 15120P IR OUT (Pin 1)
 Wi-Fi LED     ───┤ D2                        D12 ├─── Power LED (MTDI safe)
 Status LED 1  ───┤ D4                        D14 ├─── Relay 4 (IN4)
 Button 4      ───┤ RX2                       D27 ├─── Relay 3 (IN3)
 Reset / AP    ───┤ TX2                       D26 ├─── Relay 2 (IN2)
 Reserved SPI  ───┤ D5                        D25 ├─── Relay 1 (IN1)
 Reserved SPI  ───┤ D18                       D33 ├─── Status LED 4
 Reserved SPI  ───┤ D19                       D32 ├─── Status LED 3
 Reserved I2C  ───┤ D21                       D35 ├─── Button 1 (+ 10k pull-up)
 Reserved UART ───┤ RX0                       D34 ├─── LDR Divider (ADC1_CH6)
 Reserved UART ───┤ TX0                        VN ├─── Button 3 (+ 10k pull-up)
 Reserved I2C  ───┤ D22                        VP ├─── Button 2 (+ 10k pull-up)
 Reserved SPI  ───┤ D23                        EN ├─── (Reset Circuit / NC)
                  └───────────────────────────────┘
```

### Complete Pin Translation Table

| Symbol Pin Name | Physical Header Side | ESP32 GPIO Silicon Pin | System Function | Hardware Notes |
| :--- | :--- | :--- | :--- | :--- |
| **`VIN`** | Right Pin 1 | Power In | **+5V System In** | Connect to Net `+5V` (HLK Pin 4 / C1 +) |
| **`GND`** | Left Pin 2 | Ground | **System Ground** | Connect to Net `GND` |
| **`GND2`** | Right Pin 2 | Ground | **System Ground** | Connect to Net `GND` |
| **`3V3`** | Left Pin 1 | Power Out | **+3.3V Logic Rail** | Powers optos, IR sensor, LDR divider |
| **`D25`** | Right Pin 8 | GPIO 25 | **Relay 1 Control** | Active-LOW to Relay `IN1` |
| **`D26`** | Right Pin 7 | GPIO 26 | **Relay 2 Control** | Active-LOW to Relay `IN2` |
| **`D27`** | Right Pin 6 | GPIO 27 | **Relay 3 Control** | Active-LOW to Relay `IN3` |
| **`D14`** | Right Pin 5 | GPIO 14 | **Relay 4 Control** | Active-LOW to Relay `IN4` |
| **`D35`** | Right Pin 11| GPIO 35 | **Button 1 (Manual)** | Momentary to GND + **Ext 10kΩ pull-up to 3V3** |
| **`VP`**  | Right Pin 14| GPIO 36 | **Button 2 (Manual)** | Momentary to GND + **Ext 10kΩ pull-up to 3V3** |
| **`VN`**  | Right Pin 13| GPIO 39 | **Button 3 (Manual)** | Momentary to GND + **Ext 10kΩ pull-up to 3V3** |
| **`RX2`** | Left Pin 6  | GPIO 16 | **Button 4 (Manual)** | Momentary to GND (Internal `INPUT_PULLUP`) |
| **`TX2`** | Left Pin 7  | GPIO 17 | **Reset / Config AP** | Momentary to GND (Internal `INPUT_PULLUP`) |
| **`D4`**  | Left Pin 5  | GPIO 4  | **Status LED 1** | $220\Omega$ to Anode, Cathode to GND |
| **`D15`** | Left Pin 3  | GPIO 15 | **Status LED 2** | $220\Omega$ to Anode, Cathode to GND (*MTDO safe*) |
| **`D32`** | Right Pin 10| GPIO 32 | **Status LED 3** | $220\Omega$ to Anode, Cathode to GND |
| **`D33`** | Right Pin 9 | GPIO 33 | **Status LED 4** | $220\Omega$ to Anode, Cathode to GND |
| **`D12`** | Right Pin 4 | GPIO 12 | **Power LED** | $330\Omega$ to Anode, Cathode to GND (*MTDI safe*) |
| **`D2`**  | Left Pin 4  | GPIO 2  | **Wi-Fi Status LED** | $220\Omega$ to Anode, Cathode to GND |
| **`D34`** | Right Pin 12| GPIO 34 | **LDR Ambient Sensor**| Midpoint of LDR & 10kΩ divider (ADC1_CH6) |
| **`D13`** | Right Pin 3 | GPIO 13 | **15120P IR Receiver**| Pin 1 (OUT) of 15120P 38kHz sensor |
| **`D21`** | Left Pin 11 | GPIO 21 | **[RESERVED] I2C SDA**| Native Wire Data header |
| **`D22`** | Left Pin 14 | GPIO 22 | **[RESERVED] I2C SCL**| Native Wire Clock header |
| **`D18`** | Left Pin 9  | GPIO 18 | **[RESERVED] SPI SCK** | VSPI Clock header |
| **`D19`** | Left Pin 10 | GPIO 19 | **[RESERVED] SPI MISO**| VSPI Master-In header |
| **`D23`** | Left Pin 15 | GPIO 23 | **[RESERVED] SPI MOSI**| VSPI Master-Out header |
| **`D5`**  | Left Pin 8  | GPIO 5  | **[RESERVED] SPI CS**  | VSPI Chip Select header |
| **`TX0`** | Left Pin 13 | GPIO 1  | **[RESERVED] UART TX**| Native Hardware Serial |
| **`RX0`** | Left Pin 12 | GPIO 3  | **[RESERVED] UART RX**| Native Hardware Serial |
| **`EN`**  | Right Pin 15| EN/RST  | **Chip Enable** | Internal RC on DevKit (leave NC or wire to RST switch)|

---

## Step 2: 4-Channel Relay Control Interface

Controls 4 appliance relays with complete galvanic isolation between the noisy 5V coil rail and the sensitive 3.3V ESP32 logic.

### 2.1 EasyEDA Part Selection
- **Signal Header (J3):** 1x6 Pin Header 2.54mm (`HDR-TH_6P-P2.54-V-M`).
- **Power/Isolation Header (J4):** 1x3 Pin Header 2.54mm (`HDR-TH_3P-P2.54-V-M`).

### 2.2 Schematic Wiring

```
   ESP32 DevKit Symbol                     Relay Module Interface
  ┌───────────────────────┐                ┌──────────────────────┐
  │                   D25 ├────────────────┤ IN1                  │
  │                   D26 ├────────────────┤ IN2                  │
  │                   D27 ├────────────────┤ IN3                  │
  │                   D14 ├────────────────┤ IN4                  │
  │                       │                │                      │
  │       3V3             ├────────────────┤ VCC (Opto Anodes)    │
  │                       │                │                      │
  │  Net: +5V             ├────────────────┤ JD-VCC (Coil Power)  │
  │  Net: GND             ├────────────────┤ GND (Coil Return)    │
  └───────────────────────┘                └──────────────────────┘
                                           ⚠️ BLUE JUMPER REMOVED!
```

### 2.3 Wiring Steps
1. Place a **1x6 connector** (Pins: `GND`, `IN1`, `IN2`, `IN3`, `IN4`, `VCC`).
   - Pin 1 (`GND`): Connect to Net `GND`.
   - Pin 2 (`IN1`): Connect to pin **`D25`** on ESP32 symbol.
   - Pin 3 (`IN2`): Connect to pin **`D26`** on ESP32 symbol.
   - Pin 4 (`IN3`): Connect to pin **`D27`** on ESP32 symbol.
   - Pin 5 (`IN4`): Connect to pin **`D14`** on ESP32 symbol.
   - Pin 6 (`VCC`): Connect to Net **`+3.3V`** *(Powers optocoupler internal LEDs)*.
2. Place a **1x3 connector** for the JD-VCC selection block (Pins: `VCC`, `JD-VCC`, `GND`).
   - Pin 1 (`VCC`): Leave NC (open) or tie to Net `+3.3V`.
   - Pin 2 (`JD-VCC`): Connect directly to Net **`+5V`**.
   - Pin 3 (`GND`): Connect directly to Net **`GND`**.

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
- **Switches (SW1–SW5):** 6x6x5mm 4-Pin Through-Hole Tactile Switch (`SW-TH_4P-L6.0-W6.0`, LCSC: `C318884`).
- **Resistors (R1–R3):** 10kΩ 1/4W Through-Hole Axial (`R-TH_AXIAL-0.4` or 0805 SMD, LCSC: `C25804`).

### 3.2 Schematic Wiring Diagram

```
         +3.3V               +3.3V               +3.3V
           │                   │                   │
         [10k] R1            [10k] R2            [10k] R3
           │                   │                   │
           ├────── D35         ├────── VP          ├────── VN
           │ (Button 1)        │ (Button 2)        │ (Button 3)
         [SW1]               [SW2]               [SW3]
           │                   │                   │
          GND                 GND                 GND

    (Pins RX2 and TX2 use ESP32 internal INPUT_PULLUP — no external resistors needed)
           │                   │
           ├────── RX2         ├────── TX2
           │ (Button 4)        │ (Reset / AP Mode)
         [SW4]               [SW5]
           │                   │
          GND                 GND
```

### 3.3 Wiring Steps
1. **Button 1 (Pin `D35`):**
   - Connect one side of SW1 to Net `GND`.
   - Connect the other side to pin **`D35`**.
   - Connect a **10kΩ resistor (R1)** between pin `D35` and Net **`+3.3V`**.
2. **Button 2 (Pin `VP`):**
   - Connect one side of SW2 to Net `GND`.
   - Connect the other side to pin **`VP`**.
   - Connect a **10kΩ resistor (R2)** between pin `VP` and Net **`+3.3V`**.
3. **Button 3 (Pin `VN`):**
   - Connect one side of SW3 to Net `GND`.
   - Connect the other side to pin **`VN`**.
   - Connect a **10kΩ resistor (R3)** between pin `VN` and Net **`+3.3V`**.
4. **Button 4 (Pin `RX2`) & Config Button (Pin `TX2`):**
   - Connect one side of SW4 and SW5 to Net `GND`.
   - Connect the other side directly to pin **`RX2`** and pin **`TX2`**.

> [!CAUTION]
> **CRITICAL PRECAUTION — Input-Only Pins (`D35`, `VP`, `VN`):**
> Pins `D35` (GPIO 35), `VP` (GPIO 36), and `VN` (GPIO 39) are **Input-Only (GPI)** on the ESP32. They **DO NOT HAVE INTERNAL PULL-UP RESISTORS**.
> If you omit the external 10kΩ resistors on `D35`, `VP`, and `VN`, those pins will float, causing the relays to switch erratically on and off!

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
   ESP32 Symbol Pin         Resistor           LED           Ground
   ────────────────────────────────────────────────────────────────
   D4  (Status 1) ────► [ 220Ω ] ────►|─ (Green) ────► GND
   D15 (Status 2) ────► [ 220Ω ] ────►|─ (Green) ────► GND  ⚡ MTDO Strapping Safe
   D32 (Status 3) ────► [ 220Ω ] ────►|─ (Green) ────► GND
   D33 (Status 4) ────► [ 220Ω ] ────►|─ (Green) ────► GND
   D12 (Power LED)────► [ 330Ω ] ────►|─ (Red)   ────► GND  ⚡ MTDI Hardware Pull-Down
   D2  (Wi-Fi LED)────► [ 220Ω ] ────►|─ (Blue)  ────► GND  ⚡ Boot Strapping Safe
```

### 4.3 Wiring Steps
1. Wire each pin (`D4`, `D15`, `D32`, `D33`, `D12`, `D2`) to the **Anode (+, longer leg)** of the LED via its series resistor.
2. Connect all LED **Cathodes (–, flat edge)** directly to Net `GND`.

> [!IMPORTANT]
> **CRITICAL PRECAUTION — Pin `D12` (MTDI) Boot-Strapping Trap:**
> `D12` (GPIO 12) is a high-risk strapping pin. If driven HIGH at boot, the ESP32 sets the flash memory voltage to 1.8V instead of 3.3V, causing a permanent boot loop!
> Wiring the Power LED as **Active-HIGH** (Pin `D12` ──► 330Ω ──► LED ──► GND) pulls `D12` solidly to `GND` through the 330Ω resistor at startup, **guaranteeing 100% reliable 3.3V flash boot**!

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
           ├───► D34 (ADC1 Channel 6)
         ┌─┴─┐
         │10k│ R10 (1% Divider Resistor)
         └─┬─┘
           │
          GND
```

### 5.3 Wiring Steps
1. Connect top terminal of LDR to Net `+3.3V`.
2. Connect bottom terminal of LDR to pin **`D34`** on your ESP32 symbol.
3. Connect a 10kΩ resistor (R10) from pin **`D34`** down to Net `GND`.

> [!WARNING]
> **CRITICAL PRECAUTION — ADC1 vs ADC2 Wi-Fi Conflict:**
> Pin **`D34`** resides on **ADC1 (Channel 6)**. ADC1 operates concurrently with Wi-Fi without issue.
> (Never move the LDR to ADC2 pins like D2, D4, D12-D15, D25-D27, because ADC2 is disabled by ESP32 silicon whenever Wi-Fi is active).

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
            └────────────► Pin D13 (on ESP32 symbol)
```

### 6.3 Wiring Steps
1. Connect **Pin 1 (OUT)** directly to pin **`D13`** on your ESP32 symbol.
2. Connect **Pin 2 (GND)** directly to Net `GND`.
3. Connect **Pin 3 (VCC)** directly to Net `+3.3V`.
4. Place a **100nF ceramic capacitor (C2)** directly across Pin 3 (+3.3V) and Pin 2 (GND) right next to the sensor footprint.

> [!CAUTION]
> **CRITICAL PRECAUTION — IR Sensor Pinout Trap:**
> The **15120P** pinout is:
> - **Pin 1 = OUT** | **Pin 2 = GND** | **Pin 3 = VCC**
> Beware of cheap clone sensors (like some VS1838 variants) that swap Pin 2 and Pin 3 (Pin 2=VCC, Pin 3=GND). Reversing VCC and GND will instantly destroy the sensitive internal photodiode IC!

---

## Step 7: Reserved Expansion & Diagnostic Headers

To make your PCB future-proof, break out the unallocated hardware communication busses to standard 2.54mm male pin headers.

### 7.1 Expansion Headers Pinout Table

| Header | Bus Type | Pin 1 | Pin 2 | Pin 3 | Pin 4 | Pin 5 | Pin 6 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **J5 (1x4)** | **I2C** (OLED / BME280) | `+3.3V` | `GND` | **`D22`** (SCL) | **`D21`** (SDA) | — | — |
| **J6 (1x6)** | **VSPI** (SD Card / TFT) | `+3.3V` | `GND` | **`D18`** (SCK) | **`D19`** (MISO)| **`D23`** (MOSI)| **`D5`** (CS) |
| **J7 (1x3)** | **UART0** (Serial Debug) | `GND`   | **`TX0`** (TX)  | **`RX0`** (RX)  | — | — | — |

---

## 📋 Comprehensive Bill of Materials (BOM) Table

All components utilize standard through-hole footprints available in the **EasyEDA System Library**:

| Designator | Component Description | Value / Rating | EasyEDA Footprint | LCSC Part # |
| :--- | :--- | :--- | :--- | :--- |
| **PS1** | Hi-Link Isolated AC-DC SMPS | 5V 1A (5W) | `TH_HLK-5M05` | `C209907` |
| **C1** | Radial Aluminum Electrolytic | 1000µF 16V (Low ESR) | `CAP-D10.0×F5.0` | `C12328` |
| **C2** | Ceramic MLCC Decoupling | 100nF 50V X7R | `CAP-TH_D5.0-P2.50` | `C49678` |
| **U1** | ESP32 NodeMCU V1 Socket | 30-Pin (`ESP32-DEVKIT-V1-30PIN1LAB`) | `HDR-TH_15P-P2.54-V-F` | `C22550` |
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
- [ ] **Input-Only Resistors:** Confirm pins **`D35`**, **`VP`**, and **`VN`** have their 10kΩ pull-up resistors connected to `+3.3V`.
- [ ] **LDR ADC Channel:** Confirm the LDR divider enters pin **`D34`** (ADC1).
- [ ] **Relay VCC:** Confirm the relay control header pin labeled `VCC` is tied to `+3.3V`, and `JD-VCC` is tied to `+5V`.
- [ ] **Power LED on D12:** Confirm pin **`D12`** is wired to the Anode with Cathode to `GND` via 330Ω (guarantees safe MTDI boot).

---

## Step 8: Optimal 2D PCB Component Placement & Routing Architecture

![Optimal 2D PCB Component Placement](images/2D_PCB_VIEW.jpg)

### 8.1 Why the Previous Draft Needed Re-Architecting
In the initial unrouted layout:
1. **Severe High-Voltage Contamination:** 230V AC traces crossed near low-voltage sensitive pins without clearance or physical separation.
2. **Trapped Interior Connectors:** `JST1`–`JST5` were bunched inside the PCB. Plugging in external wiring harnesses would cause 20+ cables to drape directly over the ESP32, SMPS, and analog lines, picking up electromagnetic interference (EMI) and blocking physical access.
3. **Crisscrossed High-Speed Busses:** Expansion headers for SPI (`D5`, `D18`, `D19`, `D23`) and I2C/UART (`D21`, `D22`, `TX0`, `RX0`) were positioned on the right edge, forcing high-speed digital lines to snake all the way across the board underneath power components.
4. **Obstructed USB & Antenna:** The ESP32 USB port and 2.4 GHz PCB antenna were oriented toward the board interior, risking poor Wi-Fi reception and mechanical interference when flashing code.

---

### 8.2 The 4-Quadrant Functional Floorplan

To achieve an industrial-grade, visually appealing, and zero-headache PCB, the board is partitioned into **4 balanced functional quadrants**:

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│  [ZONE 1: HIGH VOLTAGE & REGULATION (Top-Left)] │  [ZONE 2: RELAY POWER & CONTROL (Top-Right)] │
│                                                 │                                              │
│   230V-AC-IN  │ 5mm │  HLK-5M05 (U1)    C1      │    CN1: JD-VCC (+5V)    JST1: 4-RELAYS       │
│   [ 2-Pin ]   │ Air │  ┌──────────┐   1000µF    │    [ 2-Pin Header ]     [ 6-Pin Connector ]  │
│               │ Slot│  │          │   (Can)     │                         (Opto Active-LOW)    │
│   5V-DC-IN    │     │  │          │     │       │                                              │
│   [ 2-Pin ]   │     │  └──────────┘    [J1]     │    Relay Optocoupler Isolation Architecture  │
│   (Aux Bench) │     │                  Selector │    100% Galvanic Separation between Coils/MCU│
├─────────────────────────────────────────────────┼──────────────────────────────────────────────┤
│  [ZONE 3: CORE MCU & BUSSES (Bottom-Left)]      │  [ZONE 4: SENSORS & USER I/O (Bottom-Right)] │
│                                                 │                                              │
│               ┌──────────────────┐              │    JST5: IR & LDR        JST3: RELAY LEDS    │
│               │   [Micro-USB]    │              │    [ 6-Pin Connector ]   [ 6-Pin ] + R4-R7   │
│   JST6        │                  │   BANK A     │                                              │
│  (I2C/UART)   │     ESP32        │   +3.3V Taps │    JST2: 4-SWITCHES      JST4: PWR & WI-FI   │
│               │    DevKit        │   (CN2-CN4)  │    [ 6-Pin ] + R1-R3     [ 4-Pin ] + R8-R9   │
│   JST7        │   (30-Pin)       │   3x 2-Pin   │    (10k Pull-ups)        (Series Resistors)  │
│   (VSPI)      │                  │              │                                              │
│               └──────────────────┘              │    BANK B: +5.0V TAPS    EASYEDA SPECS BOX   │
│   KEY1 (Reset / AP Switch)                      │    (CN5, CN6, CN7)       182mm × 96mm FR-4   │
└────────────────────────────────────────────────────────────────────────────────────────┘
```

---

### 8.3 Complete 24-Component Placement Inventory (By Quadrant)

| Designator | Component Name | Quadrant Zone | Board Location | Orientation / Pin Alignment Strategy |
| :--- | :--- | :--- | :--- | :--- |
| **`230V-AC-IN`** | KF301-2P Terminal | Zone 1 (Top-Left) | Left Outer Perimeter | Wire entry facing left. ≥5.0mm milled creepage slot isolates it from low voltage. |
| **`U1`** | HLK-5M05 SMPS | Zone 1 (Top-Left) | Center of Zone 1 | AC pins face left toward terminal; DC pins face right toward C1. |
| **`C1`** | 1000µF 16V Capacitor | Zone 1 (Top-Left) | Right side of Zone 1 | `CAP-D10.0×F5.0` reservoir immediately beside +5V and GND outputs. |
| **`5V-DC-IN`** | KF301-2P Terminal | Zone 1 (Top-Left) | Lower-Left of Zone 1 | Auxiliary bench power input facing outer perimeter. |
| **`J1`** | 1x3 Pin Header | Zone 1 (Top-Left) | Next to 5V-DC-IN | Jumper selects between HLK internal mains supply or external bench 5V. |
| **`CN1`** | 2-Pin JST Header | Zone 2 (Top-Right)| Left side of Zone 2 | Dedicated JD-VCC +5V isolated relay coil supply tap. |
| **`JST1`** | 6-Pin JST Connector | Zone 2 (Top-Right)| Center of Zone 2 | Plugs face upward/right. Aligned with right header pins `D14`, `D27`, `D26`, `D25`. |
| **`JST6`** | 6-Pin JST Connector | Zone 3 (Bottom-Left)| Upper-Left Perimeter | Aligned directly with ESP32 left header (`3V3`, `D21`, `D22`, `TX0`, `RX0`, `GND`). |
| **`JST7`** | 6-Pin JST Connector | Zone 3 (Bottom-Left)| Mid-Left Perimeter | Aligned directly with ESP32 SPI pins (`3V3`, `D5`, `D18`, `D19`, `D23`, `GND`). |
| **`KEY1`** | Tactile Pushbutton | Zone 3 (Bottom-Left)| Lower-Left Corner | Accessible reset / config AP button wired to `TX2` (GPIO 17) and `GND`. |
| **`U1-MCU`** | ESP32 NodeMCU V1 | Zone 3 (Bottom-Left)| Center of Zone 3 | Dual 1x15 female sockets. USB port oriented toward top perimeter gap. |
| **`CN2–CN4`** | 3x 2-Pin JST Headers| Zone 3 (Bottom-Left)| Right side of Zone 3 | Bank A: Dedicated +3.3V & GND auxiliary expansion taps. |
| **`JST5`** | 6-Pin JST Connector | Zone 4 (Bottom-Right)| Upper-Left of Zone 4 | Connects remote IR demodulator (`D13`) and ambient LDR divider (`D34`). |
| **`JST2`** | 6-Pin JST Connector | Zone 4 (Bottom-Right)| Mid-Left of Zone 4 | Connects 4 physical wall switches (`3V3`, `D35`, `VP`, `VN`, `RX2`, `GND`). |
| **`R1–R3`** | 10kΩ Axial Resistors| Zone 4 (Bottom-Right)| Next to JST2 | Mandatory external pull-up resistors to +3.3V for input-only pins `D35`, `VP`, `VN`. |
| **`JST3`** | 6-Pin JST Connector | Zone 4 (Bottom-Right)| Upper-Right of Zone 4| Facing right/upward. Connects front-panel cabinet relay indicator LEDs. |
| **`R4–R7`** | 220Ω Resistor Array | Zone 4 (Bottom-Right)| Right of JST3 | Current limiting resistors in series with `D33`, `D32`, `D15`, `D4`. |
| **`JST4`** | 4-Pin JST Connector | Zone 4 (Bottom-Right)| Mid-Right of Zone 4 | Facing right/upward. Connects Power LED (`D12`) and Wi-Fi LED (`D2`). |
| **`R8, R9`** | 330Ω & 220Ω Resistors| Zone 4 (Bottom-Right)| Right of JST4 | `R8` (330Ω) on `D12` prevents boot-latch; `R9` (220Ω) on `D2`. |
| **`CN5–CN7`** | 3x 2-Pin JST Headers| Zone 4 (Bottom-Right)| Lower-Left of Zone 4 | Bank B: Dedicated +5.0V & GND auxiliary expansion taps. |

---

### 8.4 Recommended PCB Routing & Layout Rules in EasyEDA

1. **AC Mains High-Voltage Isolation:**
   - Maintain a minimum of **3.5mm creepage/clearance** between 230V AC lines and any low-voltage copper.
   - Mill a physical **board outline cutout slot (≥ 5.0mm wide)** between the AC terminal block and low-voltage traces to prevent electrical flashover under high humidity.
   - AC trace widths: **≥ 2.5mm (100 mil)** with rounded $45^\circ$ corners.
2. **Ground Planes (Top & Bottom):**
   - Place a **GND Copper Pour** on both Top and Bottom layers.
   - Stitch top and bottom ground planes together with via fences around sensitive analog lines (`D34` LDR) and along the board perimeter.
3. **Power Trace Widths:**
   - +5V Primary Trunk (from HLK to C1, J1, and ESP32 VIN): **≥ 1.0mm – 1.5mm (40–60 mil)**.
   - +3.3V Logic Bus: **≥ 0.8mm (32 mil)**.
   - Digital Signals & LED Lines: **0.254mm (10 mil)**.
4. **Capacitor C1 Placement:**
   - C1 must be placed **immediately at the HLK output pins** so inrush currents from relay coil activations are buffered directly at the source, preventing VCC brownouts on the ESP32.
5. **No Cross-Harness Draping:**
   - All 9 JST/terminal connectors face outward along the perimeter edges. Cable bundles run straight out to enclosure panels without crisscrossing over the PCB.

