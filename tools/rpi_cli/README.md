# Raspberry Pi CLI Controller (`switch`)

Control and monitor your ESP32 4-Channel Smart Switch directly from your Raspberry Pi's command line or over SSH.

## 1. Quick Installation on Raspberry Pi

Run the installation script to make the `switch` command available globally in your terminal:

```bash
cd tools/rpi_cli
chmod +x install.sh
sudo ./install.sh
```

## 2. Command Reference

### Check Appliance & Sensor Status
```bash
switch status
```
*Output Example:*
```text
=== Smart Switch 4CH Status (smartswitch.local) ===
  Appliance 1 (Relay 1): [ON]
  Appliance 2 (Relay 2): [OFF]
  Appliance 3 (Relay 3): [ON]
  Appliance 4 (Relay 4): [OFF]

  Ambient Sensor (LDR): 1420 / 4095
  Status LED Auto-Dim: 65% (PWM 166/255)
  Last IR Remote Code: 0x00FFA25D
  Uptime:             12m 45s
```

### Turn Appliances ON / OFF
```bash
switch on 1         # Turns Relay 1 ON
switch off 2        # Turns Relay 2 OFF
switch toggle 3     # Toggles Relay 3
switch on all       # Turns all 4 channels ON
switch off all      # Turns all 4 channels OFF
```

### Raw JSON Output (For scripts / cron jobs)
```bash
switch status --json
```

### Custom ESP32 Host or IP
If mDNS (`smartswitch.local`) is not active in your network, supply the IP address:
```bash
switch --host 192.168.1.150 status
```
Or export it as an environment variable in your `~/.bashrc`:
```bash
export SWITCH_HOST="192.168.1.150"
```

## 3. Remote Access via SSH
Because your Raspberry Pi is accessible from anywhere (via SSH, Tailscale, or Cloudflare Tunnel), you can run commands from anywhere in the world:
```bash
ssh user@your-pi-ip "switch on 1"
```
