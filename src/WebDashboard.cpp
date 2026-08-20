#include "WebDashboard.h"

#include "Config.h"
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
  <title>Taktinen vetolaite</title>
  <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css">
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
    .pages { display: flex; gap: 10px; margin: 18px 0 0; }
    .page-button {
      flex: 1;
      padding: 10px;
      background: #14221e;
    }
    .page-button.active { color: var(--green); border-color: var(--green-dark); background: #1a2d26; }
    .page { display: none; }
    .page.active { display: block; }
    #heading-arrow { transform-origin: 150px 150px; pointer-events: none; }
    #bearing-arrow { transform-origin: 150px 150px; pointer-events: none; }
    .heading-line { stroke: var(--green); stroke-width: 6; stroke-linecap: round; }
    .heading-head { fill: var(--green); }
    .bearing-line { stroke: #f1c84b; stroke-width: 9; stroke-linecap: round; }
    .bearing-head { fill: #f1c84b; }
    .compass-ring { fill: none; stroke: #2d4b40; stroke-width: 2; }
    .compass-tick { stroke: #52675e; stroke-width: 2; }
    .compass-label { fill: var(--muted); font: 700 14px monospace; text-anchor: middle; }
    .bearing-buttons { display: flex; gap: 10px; margin-top: 4px; }
    .lock-button, .unlock-button { flex: 1; padding: 11px; }
    .lock-button { color: #14221e; background: var(--green); border-color: var(--green-dark); }
    .lock-button:hover { background: #6cf0a1; }
    .unlock-button { color: #fff1ef; background: #8f332f; border-color: #b34d47; }
    .unlock-button:hover { background: #b34d47; }
    #map-container { height: 340px; border: 1px solid var(--line); }
    #map-container .leaflet-container { background: var(--water); }
    .boat-marker { color: var(--green); font-size: 26px; transform-origin: 50% 50%; text-shadow: 0 0 4px #000; }
    .waypoint-buttons { display: flex; gap: 10px; margin-top: 4px; }
    .cancel-button { flex: 1; padding: 11px; color: #fff1ef; background: #8f332f; border-color: #b34d47; }
    .cancel-button:hover { background: #b34d47; }
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
    <header><h1>Taktinen vetolaite</h1><span class="status">● online</span></header>
    <nav class="pages">
      <button id="page-steering-btn" class="page-button active" type="button">STEERING</button>
      <button id="page-bearing-btn" class="page-button" type="button">BEARING LOCK</button>
      <button id="page-map-btn" class="page-button" type="button">MAP</button>
    </nav>
    <section id="page-steering" class="page active">
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
      <div class="readout"><span class="label">Speed</span><strong id="speed">0.0 kn</strong></div>
      <div class="readout"><span class="label">Latitude</span><strong id="latitude">--</strong></div>
      <div class="readout"><span class="label">Longitude</span><strong id="longitude">--</strong></div>
    </section>
    <section id="page-bearing" class="page">
      <section class="scene" aria-label="Bearing lock compass">
        <svg id="bearing-control" viewBox="0 0 300 300" role="img" aria-label="Compass with bearing lock control">
          <circle class="compass-ring" cx="150" cy="150" r="128"/>
          <line class="compass-tick" x1="150" y1="22" x2="150" y2="38"/>
          <line class="compass-tick" x1="150" y1="262" x2="150" y2="278"/>
          <line class="compass-tick" x1="22" y1="150" x2="38" y2="150"/>
          <line class="compass-tick" x1="262" y1="150" x2="278" y2="150"/>
          <text class="compass-label" x="150" y="56">N</text>
          <text class="compass-label" x="150" y="256">S</text>
          <text class="compass-label" x="250" y="156">E</text>
          <text class="compass-label" x="50" y="156">W</text>
          <g id="heading-arrow" aria-hidden="true">
            <line class="heading-line" x1="150" y1="150" x2="150" y2="70"/>
            <path class="heading-head" d="M150 46 L134 78 H166 Z"/>
          </g>
          <g id="bearing-arrow" aria-hidden="true">
            <line class="bearing-line" x1="150" y1="150" x2="150" y2="48"/>
            <path class="bearing-head" d="M150 18 L128 58 H172 Z"/>
          </g>
        </svg>
      </section>
      <div class="readout"><span class="label">Compass heading</span><strong id="bearing-heading">0°</strong></div>
      <div class="readout"><span class="label">Locked bearing</span><strong id="bearing-target">0°</strong></div>
      <div class="readout"><span class="label">Speed</span><strong id="bearing-speed">0.0 kn</strong></div>
      <div class="readout"><span class="label">Autopilot</span><strong id="bearing-status">UNLOCKED</strong></div>
      <div class="bearing-buttons">
        <button id="bearing-unlock" class="unlock-button" type="button">UNLOCK</button>
      </div>
    </section>
    <section id="page-map" class="page">
      <div id="map-container" aria-label="Lake map, tap to set destination"></div>
      <div class="readout"><span class="label">Navigation</span><strong id="nav-status">IDLE</strong></div>
      <div class="readout"><span class="label">Distance to target</span><strong id="nav-distance">-- m</strong></div>
      <div class="readout"><span class="label">Bearing to target</span><strong id="nav-bearing">--°</strong></div>
      <div class="waypoint-buttons">
        <button id="waypoint-cancel" class="cancel-button" type="button">CANCEL NAVIGATION</button>
      </div>
    </section>
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
  <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
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
    const speed = document.getElementById('speed');
    const bearingSpeed = document.getElementById('bearing-speed');
    const throttle = document.getElementById('throttle');
    const throttleValue = document.getElementById('throttle-value');
    const pageSteeringBtn = document.getElementById('page-steering-btn');
    const pageBearingBtn = document.getElementById('page-bearing-btn');
    const pageMapBtn = document.getElementById('page-map-btn');
    const pageSteering = document.getElementById('page-steering');
    const pageBearing = document.getElementById('page-bearing');
    const pageMap = document.getElementById('page-map');
    const bearingControl = document.getElementById('bearing-control');
    const headingArrow = document.getElementById('heading-arrow');
    const bearingArrow = document.getElementById('bearing-arrow');
    const bearingHeading = document.getElementById('bearing-heading');
    const bearingTargetLabel = document.getElementById('bearing-target');
    const bearingStatus = document.getElementById('bearing-status');
    const bearingUnlock = document.getElementById('bearing-unlock');
    const navStatus = document.getElementById('nav-status');
    const navDistance = document.getElementById('nav-distance');
    const navBearing = document.getElementById('nav-bearing');
    const waypointCancel = document.getElementById('waypoint-cancel');
    let currentAngle = 0;
    let targetAngle = null;
    let dragging = false;
    let bearingDragging = false;
    let bearingTarget = 0;
    let bearingLocked = false;
    let boatMarker = null;
    let waypointMarker = null;
    let map = null;
    let mapInitialized = false;
    let hasCenteredMap = false;

    function showPage(name) {
      pageSteering.classList.toggle('active', name === 'steering');
      pageBearing.classList.toggle('active', name === 'bearing');
      pageMap.classList.toggle('active', name === 'map');
      pageSteeringBtn.classList.toggle('active', name === 'steering');
      pageBearingBtn.classList.toggle('active', name === 'bearing');
      pageMapBtn.classList.toggle('active', name === 'map');
      if (name === 'map') {
        initMap();
        setTimeout(() => map && map.invalidateSize(), 50);
      }
    }

    function initMap() {
      if (mapInitialized) return;
      mapInitialized = true;
      map = L.map('map-container').setView([0, 0], 16);
      L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '© OpenStreetMap contributors'
      }).addTo(map);
      const boatIcon = L.divIcon({ className: '', html: '<div class="boat-marker">\u25B2</div>', iconSize: [26, 26], iconAnchor: [13, 13] });
      boatMarker = L.marker([0, 0], { icon: boatIcon }).addTo(map);
      map.on('click', async (event) => {
        const { lat, lng } = event.latlng;
        setWaypointMarker(lat, lng);
        await fetch(`/api/waypoint?lat=${encodeURIComponent(lat)}&lon=${encodeURIComponent(lng)}`);
      });
    }

    function setWaypointMarker(lat, lon) {
      if (!map) return;
      if (waypointMarker) {
        waypointMarker.setLatLng([lat, lon]);
      } else {
        waypointMarker = L.marker([lat, lon]).addTo(map);
      }
    }

    waypointCancel.addEventListener('click', async () => {
      await fetch('/api/waypoint/disable');
    });
    pageSteeringBtn.addEventListener('click', () => showPage('steering'));
    pageBearingBtn.addEventListener('click', () => showPage('bearing'));
    pageMapBtn.addEventListener('click', () => showPage('map'));

    function normalizeBearing(value) {
      return ((value % 360) + 360) % 360;
    }

    function drawBearing() {
      bearingArrow.style.transform = `rotate(${roundedAngle(bearingTarget)}deg)`;
      bearingTargetLabel.textContent = `${roundedAngle(bearingTarget)}°`;
      bearingStatus.textContent = bearingLocked ? 'LOCKED' : 'UNLOCKED';
    }

    bearingControl.addEventListener('pointerdown', (event) => {
      bearingDragging = true;
      bearingControl.setPointerCapture(event.pointerId);
    });

    bearingControl.addEventListener('pointermove', (event) => {
      if (!bearingDragging) return;
      const bounds = bearingControl.getBoundingClientRect();
      const x = event.clientX - (bounds.left + bounds.width / 2);
      const y = event.clientY - (bounds.top + bounds.height / 2);
      bearingTarget = normalizeBearing(Math.atan2(x, -y) * 180 / Math.PI);
      drawBearing();
    });

    bearingControl.addEventListener('pointerup', async (event) => {
      if (!bearingDragging) return;
      bearingDragging = false;
      bearingControl.releasePointerCapture(event.pointerId);
      bearingLocked = true;
      drawBearing();
      await fetch(`/api/bearing?angle=${encodeURIComponent(bearingTarget)}`);
    });

    bearingUnlock.addEventListener('click', async () => {
      bearingLocked = false;
      drawBearing();
      await fetch('/api/bearing/disable');
    });

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
        speed.textContent = `${data.speedKnots.toFixed(1)} kn`;
        bearingSpeed.textContent = `${data.speedKnots.toFixed(1)} kn`;
        arrow.style.transform = `rotate(${roundedAngle(data.angle)}deg)`;
        drawTarget();
        bearingHeading.textContent = `${roundedAngle(data.heading)}°`;
        headingArrow.style.transform = `rotate(${roundedAngle(data.heading)}deg)`;
        bearingLocked = data.bearingLocked;
        if (!bearingDragging) {
          bearingTarget = data.bearingTarget;
        }
        drawBearing();

        if (data.latitude !== null && data.longitude !== null && map) {
          boatMarker.setLatLng([data.latitude, data.longitude]);
          boatMarker.getElement().querySelector('.boat-marker').style.transform =
            `rotate(${roundedAngle(data.heading)}deg)`;
          if (!hasCenteredMap) {
            hasCenteredMap = true;
            map.setView([data.latitude, data.longitude], 16);
          }
        }
        if (data.waypointActive) {
          setWaypointMarker(data.waypointLat, data.waypointLon);
          navStatus.textContent = data.waypointHolding ? 'HOLDING' : 'NAVIGATING';
          navDistance.textContent = `${data.waypointDistance.toFixed(1)} m`;
          navBearing.textContent = `${roundedAngle(data.waypointBearing)}°`;
        } else {
          navStatus.textContent = 'IDLE';
          navDistance.textContent = '-- m';
          navBearing.textContent = '--°';
        }
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

  activeDashboard->recordClientHeartbeat();
  String response = "{\"angle\":" +
                    String(activeDashboard->steeringAngleDegrees(), 4) +
                    ",\"heading\":" +
                    String(activeDashboard->headingDegrees(), 4) +
                    ",\"gpsStatus\":\"" +
                    (activeDashboard->gpsHasFix() ? "FIX" : "NO FIX") +
                    "\",\"satellites\":" +
                    String(activeDashboard->gpsSatellites()) +
                    ",\"bearingLocked\":" +
                    (activeDashboard->bearingLockEnabled() ? "true" : "false") +
                    ",\"bearingTarget\":" +
                    String(activeDashboard->bearingLockTargetDegrees(), 4) +
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
  response += ",\"speedKnots\":" + String(activeDashboard->gpsSpeedKnots(), 2);
  response += ",\"waypointActive\":" +
              String(activeDashboard->waypointActive() ? "true" : "false") +
              ",\"waypointHolding\":" +
              (activeDashboard->waypointHolding() ? "true" : "false") +
              ",\"waypointLat\":" + String(activeDashboard->waypointLatitude(), 6) +
              ",\"waypointLon\":" + String(activeDashboard->waypointLongitude(), 6) +
              ",\"waypointDistance\":" + String(activeDashboard->waypointDistanceMeters(), 1) +
              ",\"waypointBearing\":" + String(activeDashboard->waypointBearingDegrees(), 1);
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
    bearingLockEnabled_ = false;
    waypointActive_ = false;
    waypointHolding_ = false;
    stopRequested_ = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/api/bearing", HTTP_GET, [this]() {
    if (!server.hasArg("angle")) {
      server.send(400, "application/json", "{\"error\":\"angle required\"}");
      return;
    }

    float angle = fmodf(server.arg("angle").toFloat(), 360.0f);
    if (angle < 0.0f) {
      angle += 360.0f;
    }
    bearingLockTargetDegrees_ = angle;
    bearingLockEnabled_ = true;
    targetRequested_ = false;
    waypointActive_ = false;
    waypointHolding_ = false;
    stopRequested_ = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/api/bearing/disable", HTTP_GET, [this]() {
    bearingLockEnabled_ = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/api/waypoint", HTTP_GET, [this]() {
    if (!server.hasArg("lat") || !server.hasArg("lon")) {
      server.send(400, "application/json", "{\"error\":\"lat and lon required\"}");
      return;
    }

    waypointLatitude_ = server.arg("lat").toDouble();
    waypointLongitude_ = server.arg("lon").toDouble();
    waypointActive_ = true;
    waypointHolding_ = false;
    targetRequested_ = false;
    bearingLockEnabled_ = false;
    stopRequested_ = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.on("/api/waypoint/disable", HTTP_GET, [this]() {
    waypointActive_ = false;
    waypointHolding_ = false;
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

bool WebDashboard::webClientConnected() const {
  return lastClientHeartbeatMs_ != 0 &&
         millis() - lastClientHeartbeatMs_ <= WEB_CLIENT_TIMEOUT_MS;
}

void WebDashboard::setWaypointTelemetry(bool holding, float distanceMeters,
                                        float bearingDegrees) {
  waypointHolding_ = holding;
  waypointDistanceMeters_ = distanceMeters;
  waypointBearingDegrees_ = bearingDegrees;
}

void WebDashboard::recordClientHeartbeat() {
  lastClientHeartbeatMs_ = millis();
}

void WebDashboard::setSteeringAngle(float angleDegrees) {
  steeringAngleDegrees_ = angleDegrees;
}

void WebDashboard::setHeading(float headingDegrees) {
  headingDegrees_ = headingDegrees;
}

void WebDashboard::setGpsData(bool hasFix, uint32_t satellites, double latitude,
                              double longitude, double speedKnots) {
  gpsHasFix_ = hasFix;
  gpsSatellites_ = satellites;
  gpsLatitude_ = latitude;
  gpsLongitude_ = longitude;
  gpsSpeedKnots_ = speedKnots;
}
