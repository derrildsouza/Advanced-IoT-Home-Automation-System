#!/usr/bin/env bash
set -e

PORT="${1:-/dev/ttyUSB0}"
ARDUINO_CLI="/home/derril/derril/arduino-cli/bin/arduino-cli"
if ! command -v "$ARDUINO_CLI" &> /dev/null; then
    ARDUINO_CLI="arduino-cli"
fi

echo "=== Uploading Firmware to ESP32 on $PORT ==="
"$ARDUINO_CLI" upload -p "$PORT" --fqbn esp32:esp32:esp32 firmware/

echo "✓ Upload complete!"
