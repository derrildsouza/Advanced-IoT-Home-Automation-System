#pragma once

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Switch 4CH</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --card-border: #334155;
      --text: #f8fafc;
      --text-dim: #94a3b8;
      --accent: #38bdf8;
      --active: #10b981;
      --active-glow: rgba(16, 185, 129, 0.35);
      --inactive: #475569;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 1.5rem; min-height: 100vh; }
    .container { max-width: 680px; margin: 0 auto; }
    header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 1.5rem; border-bottom: 1px solid var(--card-border); padding-bottom: 1rem; }
    h1 { font-size: 1.5rem; font-weight: 700; color: var(--text); }
    .status-badge { font-size: 0.75rem; padding: 0.25rem 0.6rem; border-radius: 9999px; background: #064e3b; color: #34d399; display: flex; align-items: center; gap: 0.4rem; }
    .status-dot { width: 8px; height: 8px; border-radius: 50%; background: #34d399; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 1rem; margin-bottom: 1.5rem; }
    .card { background: var(--card-bg); border: 1px solid var(--card-border); border-radius: 1rem; padding: 1.25rem; transition: all 0.2s ease; position: relative; overflow: hidden; }
    .card.active { border-color: var(--active); box-shadow: 0 4px 20px var(--active-glow); }
    .card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 1rem; }
    .channel-title { font-size: 1.1rem; font-weight: 600; }
    .channel-sub { font-size: 0.8rem; color: var(--text-dim); }
    .toggle-btn {
      width: 100%;
      padding: 0.8rem;
      border-radius: 0.6rem;
      border: none;
      font-size: 0.95rem;
      font-weight: 600;
      cursor: pointer;
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 0.5rem;
      background: var(--inactive);
      color: var(--text);
      transition: all 0.2s ease;
    }
    .card.active .toggle-btn { background: var(--active); color: #fff; }
    .quick-actions { display: flex; gap: 0.75rem; margin-bottom: 1.5rem; }
    .btn-action { flex: 1; padding: 0.65rem; border-radius: 0.5rem; border: 1px solid var(--card-border); background: var(--card-bg); color: var(--text); font-size: 0.85rem; font-weight: 600; cursor: pointer; }
    .btn-action:hover { background: var(--card-border); }
    .info-panel { background: var(--card-bg); border: 1px solid var(--card-border); border-radius: 1rem; padding: 1.25rem; margin-bottom: 1.5rem; }
    .info-title { font-size: 0.9rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.05em; color: var(--text-dim); margin-bottom: 0.75rem; }
    .info-row { display: flex; justify-content: space-between; font-size: 0.85rem; padding: 0.4rem 0; border-bottom: 1px solid rgba(255,255,255,0.05); }
    .ir-box { font-family: monospace; background: #0b1120; padding: 0.5rem 0.8rem; border-radius: 0.4rem; color: var(--accent); }
    footer { text-align: center; font-size: 0.75rem; color: var(--text-dim); margin-top: 2rem; }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <div>
        <h1>Smart Switch 4CH</h1>
        <div style="font-size: 0.8rem; color: var(--text-dim);">ESP32 Edge Home Automation</div>
      </div>
      <div id="conn-badge" class="status-badge">
        <div class="status-dot"></div>
        <span id="conn-text">Connected</span>
      </div>
    </header>

    <div class="quick-actions">
      <button class="btn-action" onclick="setAll(true)">⚡ Turn All ON</button>
      <button class="btn-action" onclick="setAll(false)">⭕ Turn All OFF</button>
    </div>

    <div class="grid">
      <div id="card-0" class="card">
        <div class="card-header">
          <div>
            <div class="channel-title">Appliance 1</div>
            <div class="channel-sub">Relay 1 (GPIO 25)</div>
          </div>
          <span id="state-text-0" style="font-size: 0.8rem; color: var(--text-dim);">OFF</span>
        </div>
        <button class="toggle-btn" onclick="toggleRelay(0)">Toggle</button>
      </div>

      <div id="card-1" class="card">
        <div class="card-header">
          <div>
            <div class="channel-title">Appliance 2</div>
            <div class="channel-sub">Relay 2 (GPIO 26)</div>
          </div>
          <span id="state-text-1" style="font-size: 0.8rem; color: var(--text-dim);">OFF</span>
        </div>
        <button class="toggle-btn" onclick="toggleRelay(1)">Toggle</button>
      </div>

      <div id="card-2" class="card">
        <div class="card-header">
          <div>
            <div class="channel-title">Appliance 3</div>
            <div class="channel-sub">Relay 3 (GPIO 27)</div>
          </div>
          <span id="state-text-2" style="font-size: 0.8rem; color: var(--text-dim);">OFF</span>
        </div>
        <button class="toggle-btn" onclick="toggleRelay(2)">Toggle</button>
      </div>

      <div id="card-3" class="card">
        <div class="card-header">
          <div>
            <div class="channel-title">Appliance 4</div>
            <div class="channel-sub">Relay 4 (GPIO 14)</div>
          </div>
          <span id="state-text-3" style="font-size: 0.8rem; color: var(--text-dim);">OFF</span>
        </div>
        <button class="toggle-btn" onclick="toggleRelay(3)">Toggle</button>
      </div>
    </div>

    <div class="info-panel">
      <div class="info-title">Sensors & Diagnostics</div>
      <div class="info-row">
        <span>Ambient Light (ADC1 GPIO 34):</span>
        <span id="ldr-val">Loading...</span>
      </div>
      <div class="info-row">
        <span>Status LEDs Auto-Dimming:</span>
        <span id="led-brightness">Loading...</span>
      </div>
      <div class="info-row">
        <span>Last Decoded IR Remote Code:</span>
        <span id="ir-val" class="ir-box">None yet</span>
      </div>
      <div class="info-row">
        <span>System Uptime:</span>
        <span id="uptime-val">--</span>
      </div>
    </div>

    <footer>
      Advanced IoT Switch &bull; Raspberry Pi & MQTT Compatible &bull; Firmware v1.0.0
    </footer>
  </div>

  <script>
    let ws;
    function initWebSocket() {
      const loc = window.location;
      const wsUri = "ws://" + loc.hostname + ":81/";
      ws = new WebSocket(wsUri);
      
      ws.onopen = () => {
        document.getElementById('conn-text').textContent = "Live (WebSocket)";
        document.getElementById('conn-badge').style.background = "#064e3b";
      };

      ws.onclose = () => {
        document.getElementById('conn-text').textContent = "Reconnecting...";
        document.getElementById('conn-badge').style.background = "#7f1d1d";
        setTimeout(initWebSocket, 2000);
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          updateUI(data);
        } catch(e) {}
      };
    }

    function updateUI(data) {
      if (data.relays) {
        data.relays.forEach((state, i) => {
          const card = document.getElementById('card-' + i);
          const txt = document.getElementById('state-text-' + i);
          if (state) {
            card.classList.add('active');
            txt.textContent = "ON";
            txt.style.color = "var(--active)";
          } else {
            card.classList.remove('active');
            txt.textContent = "OFF";
            txt.style.color = "var(--text-dim)";
          }
        });
      }
      if (data.ldr !== undefined) {
        document.getElementById('ldr-val').textContent = data.ldr + " / 4095";
      }
      if (data.led_brightness !== undefined) {
        const pct = Math.round((data.led_brightness / 255) * 100);
        document.getElementById('led-brightness').textContent = pct + "% (PWM " + data.led_brightness + "/255)";
      }
      if (data.ir_code) {
        document.getElementById('ir-val').textContent = data.ir_code;
      }
      if (data.uptime !== undefined) {
        const mins = Math.floor(data.uptime / 60);
        const secs = data.uptime % 60;
        document.getElementById('uptime-val').textContent = mins + "m " + secs + "s";
      }
    }

    function toggleRelay(ch) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ cmd: "toggle", ch: ch }));
      } else {
        fetch('/api/relay/' + ch + '/toggle', { method: 'POST' });
      }
    }

    function setAll(state) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ cmd: "all", state: state }));
      } else {
        fetch('/api/relays', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ state: state })
        });
      }
    }

    window.addEventListener('load', () => {
      initWebSocket();
      fetch('/api/status').then(r => r.json()).then(updateUI).catch(()=>{});
    });
  </script>
</body>
</html>
)rawliteral";
