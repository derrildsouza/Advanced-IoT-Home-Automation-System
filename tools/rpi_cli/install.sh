#!/usr/bin/env bash
set -e

echo "=== Installing Smart Switch CLI on Raspberry Pi ==="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLI_SOURCE="$SCRIPT_DIR/switch_ctl.py"
TARGET="/usr/local/bin/switch"

chmod +x "$CLI_SOURCE"

if [ "$EUID" -ne 0 ]; then
    echo "Creating symlink using sudo..."
    sudo ln -sf "$CLI_SOURCE" "$TARGET"
else
    ln -sf "$CLI_SOURCE" "$TARGET"
fi

echo "✓ Successfully installed! You can now run 'switch' from anywhere."
echo ""
echo "Example commands:"
echo "  switch status"
echo "  switch on 1"
echo "  switch off 1"
echo "  switch toggle 2"
echo "  switch on all"
echo "  switch off all"
