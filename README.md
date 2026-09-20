# Advanced IoT Home Automation System (ESP32)

An industrial-grade, edge-resilient smart controller for 4-channel appliance switching built on the **ESP32 WROOM**. Features hardware tactile overrides, optocoupled relay isolation, auto-dimming status LEDs via LDR, line-of-sight infrared remote control, an onboard WebSocket web dashboard, and bidirectional command-line and MQTT integration for Raspberry Pi.

---

## System Architecture

```
[Remote User / Anywhere]
        │ SSH / Tailscale / Tunnel
        ▼
┌───────────────────────────────────────────────────────────┐
│                    Raspberry Pi Server                    │
│  • CLI Tool (`switch on 1`, `switch status`, etc.)        │
│  • Mosquitto MQTT Broker (port 1883)                      │
│  • Home Assistant Auto-Discovery Support                  │
└─────────────────────────────┬─────────────────────────────┘
                              │ Wi-Fi (MQTT / HTTP / WebSockets)
                              ▼
┌───────────────────────────────────────────────────────────┐
│                 ESP32 Microcontroller                     │
│  ├── Core 0: Network Task (HTTP Server, WebSockets, MQTT) │
│  └── Core 1: Hardware Task (Relays, Buttons, IR, LDR PWM) │
└─────────────────────────────┬─────────────────────────────┘
                              │ Isolated GPIOs
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                     ▼
 4-Channel Relays      4 Physical Keys        4 Status LEDs (PWM)
 (Active LOW Opto)    (Zero-latency Int)      (LDR Auto-Dimming)
```

---

## Hardware Pinout Matrix

| Peripheral | ESP32 GPIO | Mode / Type | Description |
| :--- | :--- | :--- | :--- |
| **Relay 1** | `GPIO 25` | Output (Active LOW) | Optocoupled Relay Channel 1 |
| **Relay 2** | `GPIO 26` | Output (Active LOW) | Optocoupled Relay Channel 2 |
| **Relay 3** | `GPIO 27` | Output (Active LOW) | Optocoupled Relay Channel 3 |
| **Relay 4** | `GPIO 14` | Output (Active LOW) | Optocoupled Relay Channel 4 |
| **Button 1** | `GPIO 16` | `INPUT_PULLUP` | Tactile Button 1 to GND |
| **Button 2** | `GPIO 17` | `INPUT_PULLUP` | Tactile Button 2 to GND |
| **Button 3** | `GPIO 18` | `INPUT_PULLUP` | Tactile Button 3 to GND |
| **Button 4** | `GPIO 23` | `INPUT_PULLUP` | Tactile Button 4 to GND |
| **Status LED 1** | `GPIO 4` | LEDC PWM (Ch 0) | Auto-dimmed feedback LED 1 |
| **Status LED 2** | `GPIO 5` | LEDC PWM (Ch 1) | Auto-dimmed feedback LED 2 |
| **Status LED 3** | `GPIO 21` | LEDC PWM (Ch 2) | Auto-dimmed feedback LED 3 |
| **Status LED 4** | `GPIO 22` | LEDC PWM (Ch 3) | Auto-dimmed feedback LED 4 |
| **IR Receiver** | `GPIO 13` | Digital In / INT | 38kHz IR (VS1838B / TSOP) |
| **Ambient LDR** | `GPIO 34` | ADC1_CH6 | Voltage divider (works with Wi-Fi) |
| **Config/Reset** | `GPIO 35` | Digital In | Short = Reboot; Hold 5s = AP Mode |
| **On-Board LED** | `GPIO 2`  | Digital Out | Solid ON when Wi-Fi connected; OFF when disconnected |


Detailed schematics and isolation rules: [`docs/PINOUT_AND_SCHEMATICS.md`](docs/PINOUT_AND_SCHEMATICS.md).

---

## Quick Start

### 1. Compile Firmware
Use the included compilation script:
```bash
./build.sh
```

### 2. Upload to ESP32
Connect your ESP32 board via USB and run:
```bash
./upload.sh /dev/ttyUSB0
```

### 3. Wi-Fi Configuration
* Edit `firmware/config.h` with your Wi-Fi and Raspberry Pi IP before flashing, **or**
* If credentials fail or are unset, connect to the ESP32's hotspot `SmartSwitch-Setup` (password: `12345678`) and configure your network via the captive portal.

---

## Embedded Web Dashboard

Once connected to your local network, open any browser on your phone, tablet, or PC:
```
http://smartswitch.local
```
* **Real-time WebSockets:** Instant UI sync when physical buttons or IR remote buttons are pressed.
* **Sensor readouts:** Live LDR light levels, status LED PWM brightness, and uptime.
* **IR Code Monitor:** Shows hex codes of any remote control pointed at the sensor.

---

## Raspberry Pi CLI Controller (`switch`)

To control the switch from your Raspberry Pi terminal:

```bash
cd tools/rpi_cli
sudo ./install.sh
```

### Examples:
```bash
switch status         # Prints formatted status table of all relays & sensors
switch on 1           # Turns Appliance 1 ON
switch off 2          # Turns Appliance 2 OFF
switch toggle 3       # Toggles Appliance 3
switch on all         # Turns all appliances ON
switch off all        # Turns all appliances OFF
switch status --json  # Machine-readable output for scripts / cron
```

Full API and MQTT topic documentation: [`docs/API_AND_MQTT.md`](docs/API_AND_MQTT.md).
