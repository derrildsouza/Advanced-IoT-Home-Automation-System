#!/usr/bin/env bash
set -e

ARDUINO_CLI="/home/derril/derril/arduino-cli/bin/arduino-cli"
if ! command -v "$ARDUINO_CLI" &> /dev/null; then
    ARDUINO_CLI="arduino-cli"
fi

echo "=== Compiling Advanced IoT Home Automation Firmware ==="
"$ARDUINO_CLI" compile --fqbn esp32:esp32:esp32 firmware/

echo "✓ Build successful!"
