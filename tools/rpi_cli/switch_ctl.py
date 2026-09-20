#!/usr/bin/env python3
"""
Advanced IoT Switch - Raspberry Pi CLI Controller
Supports bidirectional control via REST API (HTTP) and MQTT broker.
"""

import sys
import os
import time
import json
import argparse
import urllib.request
import urllib.error

# Environment / Defaults
DEFAULT_HOST = os.getenv("SWITCH_HOST", "smartswitch.local")
DEFAULT_MQTT_BROKER = os.getenv("MQTT_BROKER", "localhost")
DEFAULT_MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))

# ANSI Color codes for clean terminal output
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
CYAN = "\033[96m"
BOLD = "\033[1m"
RESET = "\033[0m"


def http_get(endpoint, host=DEFAULT_HOST, timeout=3.0):
    url = f"http://{host}{endpoint}"
    req = urllib.request.Request(url)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as response:
            return json.loads(response.read().decode())
    except Exception as e:
        return {"error": str(e)}


def http_post(endpoint, data=None, host=DEFAULT_HOST, timeout=3.0):
    url = f"http://{host}{endpoint}"
    payload = json.dumps(data).encode() if data else b""
    headers = {"Content-Type": "application/json"} if data else {}
    req = urllib.request.Request(url, data=payload, headers=headers, method="POST")
    try:
        with urllib.request.urlopen(req, timeout=timeout) as response:
            return json.loads(response.read().decode())
    except Exception as e:
        return {"error": str(e)}


def cmd_status(args):
    data = http_get("/api/status", host=args.host)
    if "error" in data:
        print(f"{RED}[ERROR] Failed to reach ESP32 at {args.host}:{RESET} {data['error']}")
        sys.exit(1)

    if args.json:
        print(json.dumps(data, indent=2))
        return

    relays = data.get("relays", [False, False, False, False])
    ldr = data.get("ldr", 0)
    brightness = data.get("led_brightness", 0)
    uptime_sec = data.get("uptime", 0)
    ir_code = data.get("ir_code", "None")

    mins = uptime_sec // 60
    secs = uptime_sec % 60
    bright_pct = round((brightness / 255) * 100)

    print(f"\n{BOLD}{CYAN}=== Smart Switch 4CH Status ({args.host}) ==={RESET}")
    for i, state in enumerate(relays):
        status_label = f"{GREEN}[ON]{RESET}" if state else f"{RED}[OFF]{RESET}"
        print(f"  Appliance {i+1} (Relay {i+1}): {status_label}")

    print(f"\n  {BOLD}Ambient Sensor (LDR):{RESET} {ldr} / 4095")
    print(f"  {BOLD}Status LED Auto-Dim:{RESET} {bright_pct}% (PWM {brightness}/255)")
    print(f"  {BOLD}Last IR Remote Code:{RESET} {ir_code}")
    print(f"  {BOLD}Uptime:{RESET}             {mins}m {secs}s\n")


def cmd_on(args):
    target = args.channel.lower()
    if target == "all":
        res = http_post("/api/relays", data={"state": True}, host=args.host)
        print(f"{GREEN}✓ Turned ALL appliances ON{RESET}")
    else:
        ch = int(target) - 1
        res = http_post(f"/api/relay/{ch}?state=on", host=args.host)
        print(f"{GREEN}✓ Turned Appliance {args.channel} ON{RESET}")


def cmd_off(args):
    target = args.channel.lower()
    if target == "all":
        res = http_post("/api/relays", data={"state": False}, host=args.host)
        print(f"{YELLOW}✓ Turned ALL appliances OFF{RESET}")
    else:
        ch = int(target) - 1
        res = http_post(f"/api/relay/{ch}?state=off", host=args.host)
        print(f"{YELLOW}✓ Turned Appliance {args.channel} OFF{RESET}")


def cmd_toggle(args):
    ch = int(args.channel) - 1
    res = http_post(f"/api/relay/{ch}/toggle", host=args.host)
    print(f"{CYAN}✓ Toggled Appliance {args.channel}{RESET}")


def cmd_reboot(args):
    print(f"{YELLOW}Sending reboot signal to ESP32...{RESET}")
    http_post("/api/reboot", host=args.host)
    print("Reboot dispatched.")


def cmd_mqtt_pub(topic, message, broker=DEFAULT_MQTT_BROKER, port=DEFAULT_MQTT_PORT):
    try:
        import paho.mqtt.publish as publish
        publish.single(topic, message, hostname=broker, port=port)
        return True
    except ImportError:
        # Fallback to mosquitto_pub shell command
        ret = os.system(f"mosquitto_pub -h {broker} -p {port} -t '{topic}' -m '{message}' >/dev/null 2>&1")
        return ret == 0


def main():
    parser = argparse.ArgumentParser(
        prog="switch",
        description="CLI Controller for Advanced IoT Home Automation System"
    )
    parser.add_argument("--host", default=DEFAULT_HOST, help=f"ESP32 Host/IP (default: {DEFAULT_HOST})")
    parser.add_argument("--json", action="store_true", help="Output status in raw JSON format")

    subparsers = parser.add_subparsers(dest="command", help="Available subcommands")

    # status
    p_status = subparsers.add_parser("status", help="Get live appliance & sensor status")
    p_status.set_defaults(func=cmd_status)

    # on
    p_on = subparsers.add_parser("on", help="Turn ON an appliance (1-4 or 'all')")
    p_on.add_argument("channel", help="Channel number (1, 2, 3, 4, or all)")
    p_on.set_defaults(func=cmd_on)

    # off
    p_off = subparsers.add_parser("off", help="Turn OFF an appliance (1-4 or 'all')")
    p_off.add_argument("channel", help="Channel number (1, 2, 3, 4, or all)")
    p_off.set_defaults(func=cmd_off)

    # toggle
    p_toggle = subparsers.add_parser("toggle", help="Toggle an appliance state (1-4)")
    p_toggle.add_argument("channel", help="Channel number (1, 2, 3, 4)")
    p_toggle.set_defaults(func=cmd_toggle)

    # reboot
    p_reboot = subparsers.add_parser("reboot", help="Trigger remote ESP32 restart")
    p_reboot.set_defaults(func=cmd_reboot)

    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(0)

    args = parser.parse_args()
    if hasattr(args, "func"):
        args.func(args)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
