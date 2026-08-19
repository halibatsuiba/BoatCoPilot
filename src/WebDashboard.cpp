#include "WebDashboard.h"

#include <WebServer.h>

namespace {
WebServer server(80);
WebDashboard* activeDashboard = nullptr;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Steering Monitor</title>
  <style>
    :root {
      color-scheme: dark;
      --ink: #e8f1ed;
      --muted: #8fa39b;
      --panel: #14221e;
      --line: #2d4b40;
      --water: #081714;
      --green: #54e38e;
      --green-dark: #1a8d55;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      display: grid;
      place-items: center;
      padding: 24px;
      color: var(--ink);
      background: radial-gradient(circle at 50% 0%, #1d3930, var(--water) 58%);
      font-family: Georgia, serif;
    }
    main {
      width: min(100%, 620px);
      padding: 26px;
      border: 1px solid var(--line);
      background: rgba(20, 34, 30, .94);
      box-shadow: 0 24px 70px rgba(0, 0, 0, .35);
    }
    header { display: flex; justify-content: space-between; gap: 18px; align-items: end; }
    h1 { margin: 0; font-size: clamp(1.7rem, 6vw, 2.7rem); font-weight: 400; letter-spacing: .02em; }
    .status { color: var(--green); font: 700 .72rem/1.2 monospace; text-transform: uppercase; letter-spacing: .12em; }
    .scene {
      position: relative;
      margin: 24px 0 18px;
      min-height: 420px;
      display: grid;
      place-items: center;
      overflow: hidden;
      border: 1px solid var(--line);
      background: repeating-linear-gradient(0deg, transparent 0 39px, rgba(84, 227, 142, .06) 40px), var(--water);
    }
    .scene::before, .scene::after {
      content: ""; position: absolute; width: 140%; height: 1px;
      background: rgba(84, 227, 142, .12); transform: rotate(-12deg);
    }
    .scene::after { transform: rotate(12deg); }
    svg { width: min(72%, 330px); position: relative; z-index: 1; }
    button {
      border: 1px solid var(--line);
      border-radius: 4px;
      color: var(--ink);
      font: 700 .72rem monospace;
      letter-spacing: .1em;
      cursor: pointer;
    }
    .zero-button {
      position: absolute;
      top: 18px;
      left: 50%;
      z-index: 2;
      padding: 8px 12px;
      transform: translateX(-50%);
      color: #f1c84b;
      background: #14221e;
    }
    .zero-button:hover { background: #2d4b40; }
    .stop-button {
      width: 100%;
      margin-top: 18px;
      padding: 13px;
      border-color: #b34d47;
      color: #fff1ef;
      background: #8f332f;
    }
    .stop-button:hover { background: #b34d47; }
    .hull { fill: #b7c9c0; stroke: #e8f1ed; stroke-width: 3; }
    .deck { fill: #38564b; stroke: #8da99c; stroke-width: 2; }
    .keel { fill: #1a3029; stroke: #8da99c; stroke-width: 2; }
    #arrow { transform-origin: 150px 150px; transition: transform .15s linear; }
    #target-arrow { transform-origin: 150px 150px; pointer-events: none; }
    .arrow-line { stroke: var(--green); stroke-width: 9; stroke-linecap: round; }
    .arrow-head { fill: var(--green); }
    .target-line { stroke: #f1c84b; stroke-width: 6; stroke-linecap: round; }
    .target-head { fill: #f1c84b; }
    .readout { display: flex; justify-content: space-between; align-items: baseline; gap: 16px; }
    .label { color: var(--muted); font: .75rem monospace; text-transform: uppercase; letter-spacing: .13em; }
    #angle { color: var(--green); font: 700 clamp(2rem, 9vw, 3.8rem)/1 monospace; }
    .throttle { margin-top: 24px; padding-top: 18px; border-top: 1px solid var(--line); }
    .throttle-row { display: flex; justify-content: space-between; gap: 12px; align-items: center; }
    #throttle-value { color: #f1c84b; font: 700 1.5rem monospace; }
    .throttle-control {
      position: relative;
      width: min(100%, 180px);
      height: 260px;
      margin: 18px auto 0;
      padding: 18px 0;
      display: grid;
      place-items: center;
      border: 2px solid #52675e;
      border-radius: 8px;
      background: linear-gradient(90deg, #0d1d18, #20382f 48%, #0d1d18);
      box-shadow: inset 0 0 0 5px #101d19, inset 0 0 18px rgba(0, 0, 0, .55);
    }
    .throttle-control::before, .throttle-control::after {
      position: absolute;
      left: 12px;
      color: var(--muted);
      font: 700 .65rem monospace;
      letter-spacing: .08em;
    }
    .throttle-control::before { content: "FWD"; top: 8px; }
    .throttle-control::after { content: "REV"; bottom: 8px; }
    #throttle {
      width: 220px;
      height: 42px;
      margin: 0;
      transform: rotate(-90deg);
      appearance: none;
      background: transparent;
      cursor: ns-resize;
    }
    #throttle::-webkit-slider-runnable-track {
      height: 10px;
      border: 1px solid #668075;
      border-radius: 5px;
      background: linear-gradient(90deg, #a94b45 0 49%, #52675e 49% 51%, #54e38e 51%);
    }
    #throttle::-webkit-slider-thumb {
      width: 54px;
      height: 30px;
      margin-top: -11px;
      border: 2px solid #f7d66d;
      border-radius: 5px;
      background: #d6a928;
      box-shadow: 0 2px 5px rgba(0, 0, 0, .5);
      appearance: none;
    }
    #throttle::-moz-range-track {
      height: 10px;
      border: 1px solid #668075;
      border-radius: 5px;
      background: linear-gradient(90deg, #a94b45 0 49%, #52675e 49% 51%, #54e38e 51%);
    }
    #throttle::-moz-range-thumb {
      width: 54px;
      height: 30px;
      border: 2px solid #f7d66d;
      border-radius: 5px;
      background: #d6a928;
      box-shadow: 0 2px 5px rgba(0, 0, 0, .5);
    }
    .throttle-scale {
      position: absolute;
      right: 12px;
      top: 50%;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      height: 224px;
      transform: translateY(-50%);
      color: var(--muted);
      font: .62rem monospace;
    }
    .note { margin: 12px 0 0; color: var(--muted); font: .78rem monospace; }
  </style>
</head>
<body>
  <main>
    <header><h1>Steering monitor</h1><span class="status">● online</span></header>
    <section class="scene" aria-label="Boat steering direction">
      <button id="zero" class="zero-button" type="button">ZERO</button>
      <svg id="steering-control" viewBox="0 0 300 300" role="img" aria-label="Top-down boat with steering control">
        <path class="hull" d="M150 18 C205 50 238 122 226 199 C217 250 186 278 150 288 C114 278 83 250 74 199 C62 122 95 50 150 18Z"/>
        <path class="deck" d="M150 48 C179 73 193 111 191 159 L109 159 C107 111 121 73 150 48Z"/>
        <path class="keel" d="M111 166 H189 V222 C179 247 164 261 150 267 C136 261 121 247 111 222Z"/>
        <g id="arrow" aria-hidden="true">
          <line class="arrow-line" x1="150" y1="222" x2="150" y2="93"/>
          <path class="arrow-head" d="M150 57 L126 103 H174 Z"/>
        </g>
        <g id="target-arrow" aria-hidden="true">
          <line class="target-line" x1="150" y1="222" x2="150" y2="108"/>
          <path class="target-head" d="M150 78 L132 112 H168 Z"/>
        </g>
      </svg>
    </section>
    <div class="readout"><span class="label">Steering axle angle</span><strong id="angle">0°</strong></div>
    <div class="readout"><span class="label">Compass heading</span><strong id="heading">0°</strong></div>
    <div class="readout"><span class="label">GPS status</span><strong id="gps-status">NO FIX</strong></div>
    <div class="readout"><span class="label">Satellites</span><strong id="satellites">0</strong></div>
    <div class="readout"><span class="label">Latitude</span><strong id="latitude">--</strong></div>
    <div class="readout"><span class="label">Longitude</span><strong id="longitude">--</strong></div>
    <button id="stop" class="stop-button" type="button">STOP</button>
    <section class="throttle">
      <div class="throttle-row"><span class="label">Throttle</span><strong id="throttle-value">0%</strong></div>
      <div class="throttle-control">
        <input id="throttle" type="range" min="-100" max="100" value="0" step="1" aria-label="Throttle">
        <div class="throttle-scale" aria-hidden="true"><span>100</span><span>50</span><span>N</span><span>50</span><span>100</span></div>
      </div>
    </section>
    <p class="note">Live AS5600 position · updates every 100 ms</p>
  </main>
  <script>
    const angle = document.getElementById('angle');
    const arrow = document.getElementById('arrow');
    const targetArrow = document.getElementById('target-arrow');
    const control = document.getElementById('steering-control');
    const zero = document.getElementById('zero');
    const stop = document.getElementById('stop');
    const heading = document.getElementById('heading');
    const gpsStatus = document.getElementById('gps-status');
    const satellites = document.getElementById('satellites');
    const latitude = document.getElementById('latitude');
    const longitude = document.getElementById('longitude');
    const throttle = document.getElementById('throttle');
    const throttleValue = document.getElementById('throttle-value');
    let currentAngle = 0;
    let targetAngle = null;
    let dragging = false;

    function showThrottle(value) {
      throttle.value = value;
      throttleValue.textContent = `${value}%`;
    }

    function roundedAngle(value) {
      return Math.round(value);
    }

    throttle.addEventListener('input', () => {
      showThrottle(throttle.value);
    });

    throttle.addEventListener('change', async () => {
      await fetch(`/api/throttle?percent=${encodeURIComponent(throttle.value)}`);
    });

    function drawTarget() {
      if (targetAngle !== null) {
        targetArrow.style.transform = `rotate(${roundedAngle(targetAngle)}deg)`;
      }
    }

    zero.addEventListener('click', async () => {
      targetAngle = 0;
      drawTarget();
      await fetch('/api/target?angle=0');
    });

    stop.addEventListener('click', async () => {
      showThrottle(0);
      await fetch('/api/stop');
    });

    control.addEventListener('pointerdown', (event) => {
      dragging = true;
      control.setPointerCapture(event.pointerId);
    });

    control.addEventListener('pointermove', (event) => {
      if (!dragging) return;
      const bounds = control.getBoundingClientRect();
      const x = event.clientX - (bounds.left + bounds.width / 2);
      const y = event.clientY - (bounds.top + bounds.height / 2);
      const pointerAngle = Math.atan2(x, -y) * 180 / Math.PI;
      const targetOffset = Math.max(-90, Math.min(90, pointerAngle));
      targetAngle = currentAngle + targetOffset;
      drawTarget();
    });

    control.addEventListener('pointerup', async (event) => {
      if (!dragging) return;
      dragging = false;
      control.releasePointerCapture(event.pointerId);
      await fetch(`/api/target?angle=${encodeURIComponent(targetAngle)}`);
    });

    async function refresh() {
      try {
        const response = await fetch('/api/steering', { cache: 'no-store' });
        const data = await response.json();
        currentAngle = data.angle;
        if (targetAngle === null) {
          targetAngle = currentAngle;
        }
        angle.textContent = `${roundedAngle(data.angle)}°`;
        heading.textContent = `${roundedAngle(data.heading)}°`;
        gpsStatus.textContent = data.gpsStatus;
        satellites.textContent = data.satellites;
        latitude.textContent = data.latitude === null ? '--' : data.latitude.toFixed(6);
        longitude.textContent = data.longitude === null ? '--' : data.longitude.toFixed(6);
        arrow.style.transform = `rotate(${roundedAngle(data.angle)}deg)`;
        drawTarget();
      } catch (_) {}
    }
    refresh();
    setInterval(refresh, 100);
  </script>
</body>
</html>
)rawliteral";

void serveIndex() {
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void serveSteering() {
  if (activeDashboard == nullptr) {
    server.send(503, "application/json", "{\"error\":\"unavailable\"}");
    return;
  }

  String response = "{\"angle\":" +
                    String(activeDashboard->steeringAngleDegrees(), 4) +
                    ",\"heading\":" +
                    String(activeDashboard->headingDegrees(), 4) +
                    ",\"gpsStatus\":\"" +
                    (activeDashboard->gpsHasFix() ? "FIX" : "NO FIX") +
                    "\",\"satellites\":" +
                    String(activeDashboard->gpsSatellites()) +
                    ",\"latitude\":";
  if (activeDashboard->gpsHasFix()) {
    response += String(activeDashboard->gpsLatitude(), 6);
  } else {
    response += "null";
  }
  response += ",\"longitude\":";
  if (activeDashboard->gpsHasFix()) {
    response += String(activeDashboard->gpsLongitude(), 6);
  } else {
    response += "null";
  }
  response += "}";
  server.send(200, "application/json", response);
}
}  // namespace

void WebDashboard::begin(uint16_t port) {
  (void)port;
  activeDashboard = this;
  server.on("/", HTTP_GET, serveIndex);
  server.on("/api/steering", HTTP_GET, serveSteering);
  server.on("/api/target", HTTP_GET, [this]() {
    if (!server.hasArg("angle")) {
      server.send(400, "application/json", "{\"error\":\"angle required\"}");
      return;
    }

    targetAngleDegrees_ = server.arg("angle").toFloat();
    targetRequested_ = true;
    stopRequested_ = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/api/throttle", HTTP_GET, [this]() {
    if (!server.hasArg("percent")) {
      server.send(400, "application/json", "{\"error\":\"percent required\"}");
      return;
    }

    throttlePercent_ = constrain(server.arg("percent").toInt(), -100, 100);
    stopRequested_ = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/api/stop", HTTP_GET, [this]() {
    throttlePercent_ = 0;
    stopRequested_ = true;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.begin();
}

void WebDashboard::handleClient() {
  server.handleClient();
}

void WebDashboard::setSteeringAngle(float angleDegrees) {
  steeringAngleDegrees_ = angleDegrees;
}

void WebDashboard::setHeading(float headingDegrees) {
  headingDegrees_ = headingDegrees;
}

void WebDashboard::setGpsData(bool hasFix, uint32_t satellites, double latitude,
                              double longitude) {
  gpsHasFix_ = hasFix;
  gpsSatellites_ = satellites;
  gpsLatitude_ = latitude;
  gpsLongitude_ = longitude;
}
