# API & MQTT Communication Reference

## 1. REST API Endpoints

The ESP32 exposes a fast, non-blocking HTTP REST API accessible at `http://smartswitch.local` (or `http://<ESP32_IP>`).

### 1.1 `GET /api/status`
Returns the real-time operational status of all relays, sensors, and system metrics.

**Response `(200 OK)`:**
```json
{
  "relays": [true, false, true, false],
  "ldr": 1820,
  "led_brightness": 140,
  "uptime": 3600,
  "ir_code": "0x00FFA25D"
}
```

---

### 1.2 `POST /api/relay/{0..3}/toggle`
Toggles the specified channel (`0` = Relay 1, `1` = Relay 2, `2` = Relay 3, `3` = Relay 4).

**Response `(200 OK)`:**
```json
{
  "success": true
}
```

---

### 1.3 `POST /api/relay/{0..3}?state={on|off}`
Explicitly turns a channel ON or OFF.

*Example:* `curl -X POST "http://smartswitch.local/api/relay/0?state=on"`

---

### 1.4 `POST /api/relays`
Bulk update for all 4 channels.

**Request Body:**
```json
{
  "state": false
}
```

---

### 1.5 `POST /api/reboot`
Triggers an immediate software reboot of the ESP32.

---

## 2. WebSockets Real-Time Stream

* **Port:** `81` (`ws://smartswitch.local:81/`)
* **Protocol:** JSON text frames
* **Bidirectional:**
  * Client sends `{ "cmd": "toggle", "ch": 0 }` to toggle.
  * Server broadcasts updated full state whenever any input (Button, IR, Web, MQTT) changes the system state.

---

## 3. MQTT Topic Hierarchy

The ESP32 connects to the Raspberry Pi Mosquitto broker at port `1883`.

### 3.1 Subscribed Topics (Commands to ESP32)
| Topic | Payload | Action |
| :--- | :--- | :--- |
| `home/switch/relay1/set` | `ON`, `OFF`, `TOGGLE` | Controls Relay 1 |
| `home/switch/relay2/set` | `ON`, `OFF`, `TOGGLE` | Controls Relay 2 |
| `home/switch/relay3/set` | `ON`, `OFF`, `TOGGLE` | Controls Relay 3 |
| `home/switch/relay4/set` | `ON`, `OFF`, `TOGGLE` | Controls Relay 4 |
| `home/switch/all/set` | `ON`, `OFF` | Turns all relays ON or OFF |

### 3.2 Published Topics (State from ESP32)
| Topic | Payload | Description |
| :--- | :--- | :--- |
| `home/switch/availability` | `online` / `offline` | LWT (Last Will and Testament) availability |
| `home/switch/relay1/state`| `ON` / `OFF` | Retained state of Relay 1 |
| `home/switch/relay2/state`| `ON` / `OFF` | Retained state of Relay 2 |
| `home/switch/relay3/state`| `ON` / `OFF` | Retained state of Relay 3 |
| `home/switch/relay4/state`| `ON` / `OFF` | Retained state of Relay 4 |
| `home/switch/status` | JSON object | Full telemetry (relays, LDR, brightness, uptime) |

### 3.3 Home Assistant Auto-Discovery
The firmware publishes discovery packets under `homeassistant/switch/smartswitch_relay{1..4}/config`.
When Mosquitto is connected to Home Assistant, the 4 switches will automatically populate in Home Assistant's dashboard without any YAML configuration.
