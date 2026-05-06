// Async Web Server HTML page, thanks ChatGPT.....l=l

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Receiver Control</title>
  <style>


html, body {
  height: 100%;
  margin: 0;
}

#app {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}


h2 {
  margin: 6px 0 4px 0;
}


/* wrapper */
#tuneWrap {
  position: relative;
  display: inline-block;
  width: 240px;
  margin-left: 30px;
}

/* labels */
.tuneLabel {
  position: absolute;
  top: -8px;
  font-size: 12px;
  color: #0f0;
  text-shadow: 0 0 4px #0f0;
  pointer-events: none;
}

/* alignment */
.tuneLabel.left {
  left: 7px;
}

.tuneLabel.right {
  right: 7px;
}



#scanRow {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 6px;
  width: fit-content;
  margin: 4px auto;
}


#scanBtn {
  height: 26px;
  padding: 0 8px;
  font-size: 11px;
  line-height: 1;
  white-space: nowrap;
}

#spectrum {
  width: 600px;        /* fixed desktop width */
  height: 120px;       /*  height */
  max-width: 100%;     /* shrink on mobile */
  display: block;
  background: #000;
  border: 1px solid #555;
  border-radius: 6px;
}

#sliderBox {
  display: flex;
  gap: 10px;
  align-items: center;
}

.sliderGroup {
  display: flex;
  flex-direction: column;
  align-items: center;
  font-size: 11px;
  color: #aaa;
}

/* base slider size */
.sliderGroup input {
  width: clamp(70px, 22vw, 110px);
}

.sliderGroup input[type="range"] {
  -webkit-appearance: none;
  appearance: none;
  height: 10px;
  background: #222;
  border-radius: 6px;
  outline: none;
}

/* track */
.sliderGroup input[type="range"]::-webkit-slider-runnable-track {
  height: 10px;
  background: #222;
  border-radius: 6px;
  box-shadow: 0 0 6px #0f0;
}

/* thumb */
.sliderGroup input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: #0f0;
  border: 1px solid #0a0;
  margin-top: -5px;
  box-shadow: 0 0 10px #0f0;
  cursor: pointer;
}


#memTitle {
  margin: 0 0 4px 0;
}

#memoryRow button {
  height: 20px;
  padding: 0 10px;
  font-size: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
}

@media (max-width: 480px) {
  #memoryRow button {
    font-size: 10px; /* smaller on mobile */
    padding: 4px 6px;
  }
   #scanHint {
    display: none;
  }

  /* stack tighter */
  #scanRow {
    flex-direction: column;
    gap: 2px;
    align-items: center;
  }

  /* pull button block closer to canvas */
  #scanControls {
    margin-top: 0;
  }

  /* reduce button height slightly */
  #scanBtn {
    height: 24px;
    font-size: 10px;
    padding: 0 6px;
  }


#sliderBox {
  flex-wrap: wrap;           /* allow wrapping */
  justify-content: center;
  gap: 8px;
}

.sliderGroup {
  flex: 1 1 calc(50% - 8px); /* 2 per row */
  max-width: calc(50% - 8px);
}

.sliderGroup input {
  width: 100%;               /* fill column */
}

}




#memoryBar {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  flex-wrap: wrap;
  margin-bottom: 4px;
}

#memoryRow {
  display: flex;
  flex-wrap: wrap;
  justify-content: center;
  gap: 4px;
}

/* Each button = 1/12 of row (max), but can shrink */
#memoryRow button {
  flex: 1 1 calc(100% / 12 - 4px);
  max-width: calc(100% / 12 - 4px);
  min-width: 55px;
}

#memHint {
  font-size: 11px;
  color: #aaa;
  white-space: nowrap;
}


section h3 {
  margin-top: 2px;
  margin-bottom: 2px;
}


section h2 {
  margin-top: 2px;
  margin-bottom: 2px;
}


#audioSection {
  display: flex;
  justify-content: center;
  gap: 12px;
  margin: 8px auto;
  flex-wrap: wrap;
}

#audioSection button {
  flex: 1 1 120px;
  max-width: 160px;
  padding: 6px 10px;
  font-size: 13px;
}


@media (max-width: 480px) {

  /* audio buttons */
  #audioSection button {
    flex: 1 1 45%;
    font-size: 16px;
    padding: 12px;
  }

  /* REMOVE S-meter completely */
  #signalBox,
  #smeter,
  #signalLabel {
    display: none !important;
  }

  #freqDisplay {
    transform: none;
  }

}





#recBtn {
  background: linear-gradient(#aa2222, #550000);
  color: #fff;
  border-color: #aa4444;
}

#recBtn.active {
  background: linear-gradient(#ff4444, #aa0000);
  box-shadow: 0 0 10px #ff0000;
  color: #fff;
}


#audioSection button.active {
  background: linear-gradient(#0f0, #070);
  color: #000;
  border-color: #0f0;
  box-shadow: 0 0 8px #0f0;
}

#audioSection button.inactive {
  opacity: 0.5;
}

#freqControlInput {
  font-family: Consolas, monospace;
  font-size: 18px;
  padding: 8px 12px;
  width: 220px;

  color: #0f0;
  background: #111;
  border: 2px solid #555;
  border-radius: 10px;

  text-align: center;
  letter-spacing: 1px;

  outline: none;
  transition: all 0.15s ease;
}

#freqControlInput:focus {
  border-color: #0f0;
  box-shadow: 0 0 6px #0f0;
  background: #000;
}


#mode:active, #mode:focus {
  border-color: #0f0;
  box-shadow: 0 0 5px #0f0;
}



button {
  border-radius: 10px;
  border: 1px solid #555;
  background: linear-gradient(#555, #333);
  color: #eee;
  box-shadow: 0 2px 4px rgba(0,0,0,0.6);
  transition: all 0.15s ease;
  cursor: pointer;
}

/* hover = lift */
button:hover {
  transform: translateY(-1px);
  box-shadow: 0 4px 8px rgba(0,0,0,0.8);
}

/* pressed = push */
button:active {
  transform: translateY(1px);
  box-shadow: 0 1px 2px rgba(0,0,0,0.8);
}

body {
  font-family: Consolas;
  margin: 0;
  padding: 8px 20px;
  background: #111;   /* fallback solid dark */
  color: #eee;
  text-align: center;
}
    section {
      border:1px solid #666;
      padding:8px;
      margin:6px auto;
      width:90%;
      background:#333;
      border-radius:8px;
    }


#pageTitle {
  margin: 0 0 4px 0;
  font-size: clamp(14px, 3vw, 20px);
  line-height: 1.1;
  color: #ddd;
  font-weight: normal;
}

/* -------- TOP PANEL -------- */
#topPanel {
  position: relative;
  display: flex;
  justify-content: center;   /* center freq display */
  align-items: center;
}

#modBox {
  position: absolute;
  right: 10px;
  top: 50%;
  transform: translateY(-50%);

  display: flex;
  flex-direction: column;
  align-items: center;
}

#scanControls {
  display: flex;
  flex-direction: column;
  align-items: center;
}

#scanHint {
  font-size: 10px;
  color: #aaa;
  margin-top: 2px;
  white-space: nowrap;
}



/* LEFT: Signal */
#signalBox {
  position: absolute;
  left: 10px;
}


#freqDisplay {
  margin: 0 auto;
  transform: none;
  font-size: clamp(24px, 8vw, 56px);
    color: #0f0;
  background: #000;
  border: 2px solid #555;
  border-radius: 6px;
  text-align: center;

  width: 90%;
  max-width: 560px;
  padding: 10px;
}



/* Audio buttons*/


#audioBox button {
  padding: 6px 12px;
  font-size: 13px;
  background-color: #444;
  color: #eee;
  border: 1px solid #666;
  border-radius: 4px;
  cursor: pointer;
}

#recBtn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
  box-shadow: none;
  transform: none;
}


#freqForm {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  flex-wrap: wrap;
}

/* ===== Tune slider ===== */
#tuneSlider {
  -webkit-appearance: none;
  appearance: none;

  width: 240px;
  margin-left: 0px;

  height: 8px;
  background: transparent;
  outline: none;
}

/* ===== TRACK ===== */
#tuneSlider::-webkit-slider-runnable-track {
  height: 8px;
  border-radius: 4px;

  background: linear-gradient(
    to right,
    #222 0%,
    #222 45%,
    #444 45%,
    #444 55%,   /* dead zone */
    #222 55%,
    #222 100%
  );

  box-shadow: 0 0 4px #0f0;
}

/* ===== THUMB ===== */
#tuneSlider::-webkit-slider-thumb {
  -webkit-appearance: none;

  width: 14px;
  height: 14px;
  border-radius: 50%;

  background: #0f0;
  border: 1px solid #0a0;

  margin-top: -3px;
  box-shadow: 0 0 6px #0f0;

  cursor: pointer;
}

#tuneSlider:active::-webkit-slider-thumb {
  box-shadow: 0 0 10px #0f0;
}

/* -------- S-METER -------- */

#signalLabel {
  margin-bottom: 6px;
  font-weight: bold;
  font-size: 14px;   /* fixed size */
}

#smeter {

  width: clamp(180px, 80vw, 280px);
  height: clamp(16px, 3vw, 24px);
  height: 22px;
  margin: 0 auto;
  position: relative;

  background: #000;
  border: 1px solid #555;

  border-radius: 12px;
  overflow: hidden;
}

/* the fill bar */
#bar {
  position: absolute;
  left: 0;
  top: 0;
  height: 100%;
  width: 0;

  background: green;

  border-radius: 12px;
  transition: width 0.2s ease;
}




    #modLabel { margin-bottom: 6px; font-weight: bold; }
    #mode {
      padding: 6px 10px;
      font-size: 14px;
      background-color: #222;
      color: #eee;
      border: 1px solid #555;
      border-radius: 4px;
    }
    #modBox button {
      margin-top: 6px;
      padding: 6px 12px;
      font-size: 13px;
      background-color: #444;
      color: #eee;
      border: 1px solid #666;
      border-radius: 4px;
      cursor: pointer;
    }


.sendBtn {
  margin-left: 12px;
}

.stepGrid {
  display: flex;
  justify-content: center;
  gap: 10px;
  flex-wrap: wrap;
  margin: 10px 0 4px 0;
}

.stepCol {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.stepCol button {
  width: 70px;
  padding: 4px 6px;
  font-size: 12px;
  font-weight: bold;
}

/* Negative (red) */
.negStep {
  background: linear-gradient(#f6c1be, #e08c89);
  color: #4a1212;
  border: 1px solid #aa6666;
}

.negStep:hover {
  background: linear-gradient(#ffd6d3, #e89a96);
}

/* Positive (green) */
.posStep {
  background: linear-gradient(#c6efc3, #89d089);
  color: #124a12;
  border: 1px solid #66aa66;
}

.posStep:hover {
  background: linear-gradient(#d9f7d6, #9be29b);
}

  </style>
</head>
<body>
<div id="app">
<h2 id="pageTitle">WiFi interface for wide band receiver</h2>
  <section id="topPanel">
    <div id="signalBox">
      <div id="signalLabel">Signal</div>
      <div id="smeter"><div id="bar"></div></div>
    </div>
    <input id="freqDisplay" type="text" value="" readonly>



    <div id="modBox">
      <select id="mode" onchange="setMode()">
  <option value="1">AM</option>
  <option value="2">LSB</option>
  <option value="3">USB</option>
  <option value="4">NBFM</option>
  <option value="5">WBFM</option>
  <option value="6">SYNC</option>
  <option value="7">CW</option>
</select>
    </div>
  </section>

<section id="audioSection">
  <button id="audioOnBtn" onclick="startAudio()">Audio ON</button>
  <button id="audioOffBtn" onclick="stopAudio()">Audio OFF</button>
  <button id="recBtn" onclick="toggleRecording()">REC</button>

<div id="sliderBox">

<div class="sliderGroup">
  <label for="volSlider">VOLUME</label>
  <input id="volSlider" type="range" min="0" max="100" value="70">
</div>

<div class="sliderGroup">
  <label for="sqlSlider">SQUELCH</label>
  <input id="sqlSlider" type="range" min="0" max="100" value="0">
</div>

<div class="sliderGroup">
   <label for="fadeSlider">NOISEGATE</label>
   <input id="fadeSlider" type="range" min="0" max="100" value="0">
</div>

<div class="sliderGroup">
  <label for="clickSlider">PULSE BLANKER</label>
  <input id="clickSlider" type="range" min="0" max="100" value="0">
</div>


</div>

</section>
  <section>
    <h2>Frequency Control</h2>
     <form id="freqForm" onsubmit="setFreq(); return false;">
     <input id="freqControlInput" type="text" inputmode="numeric" placeholder="Enter in KHz">
     <button type="submit" class="sendBtn">Send</button>

   <div id="tuneWrap">
  <span class="tuneLabel left">-</span>
  <input id="tuneSlider" type="range" min="-100" max="100" value="0">
  <span class="tuneLabel right">+</span>
</div>

</form>


<div class="stepGrid">

    <div class="stepCol">
    <button class="posStep" onmousedown="startStep(10)" onmouseup="stopStep()" onmouseleave="stopStep()">+10 Hz</button>
    <button class="negStep" onmousedown="startStep(-10)" onmouseup="stopStep()" onmouseleave="stopStep()">-10 Hz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(50)" onmouseup="stopStep()" onmouseleave="stopStep()">+50 Hz</button>
    <button class="negStep" onmousedown="startStep(-50)" onmouseup="stopStep()" onmouseleave="stopStep()">-50 Hz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(100)" onmouseup="stopStep()" onmouseleave="stopStep()">+100 Hz</button>
    <button class="negStep" onmousedown="startStep(-100)" onmouseup="stopStep()" onmouseleave="stopStep()">-100 Hz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(1000)" onmouseup="stopStep()" onmouseleave="stopStep()">+1 kHz</button>
    <button class="negStep" onmousedown="startStep(-1000)" onmouseup="stopStep()" onmouseleave="stopStep()">-1 kHz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(5000)" onmouseup="stopStep()" onmouseleave="stopStep()">+5 kHz</button>
    <button class="negStep" onmousedown="startStep(-5000)" onmouseup="stopStep()" onmouseleave="stopStep()">-5 kHz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(10000)" onmouseup="stopStep()" onmouseleave="stopStep()">+10 kHz</button>
    <button class="negStep" onmousedown="startStep(-10000)" onmouseup="stopStep()" onmouseleave="stopStep()">-10 kHz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(50000)" onmouseup="stopStep()" onmouseleave="stopStep()">+50 kHz</button>
    <button class="negStep" onmousedown="startStep(-50000)" onmouseup="stopStep()" onmouseleave="stopStep()">-50 kHz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(100000)" onmouseup="stopStep()" onmouseleave="stopStep()">+100 kHz</button>
    <button class="negStep" onmousedown="startStep(-100000)" onmouseup="stopStep()" onmouseleave="stopStep()">-100 kHz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(1000000)" onmouseup="stopStep()" onmouseleave="stopStep()">+1 MHz</button>
    <button class="negStep" onmousedown="startStep(-1000000)" onmouseup="stopStep()" onmouseleave="stopStep()">-1 MHz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(10000000)" onmouseup="stopStep()" onmouseleave="stopStep()">+10 MHz</button>
    <button class="negStep" onmousedown="startStep(-10000000)" onmouseup="stopStep()" onmouseleave="stopStep()">-10 MHz</button>
  </div>

  <div class="stepCol">
    <button class="posStep" onmousedown="startStep(100000000)" onmouseup="stopStep()" onmouseleave="stopStep()">+100MHz</button>
    <button class="negStep" onmousedown="startStep(-100000000)" onmouseup="stopStep()" onmouseleave="stopStep()">-100MHz</button>
  </div>

</div>

  </section>
    <section>

<div id="scanRow">
  <canvas id="spectrum" width="600" height="120"></canvas>
  <div id="scanControls">
    <button id="scanBtn" onclick="startScan()">SCAN</button>
    <div id="scanHint">Drop cursor on peak to listen</div>
  </div>
</div>


<h3 id="memTitle">Memory bank</h3>

<div id="memoryBar">
  <div id="memoryRow"></div>
  <span id="memHint">Memory: L=load, R=save</span>
</div>

    <h3>Amateur Bands</h3>
    <div class="presetRow">
      <button onclick="presetFreq(1800000)">160m</button>
      <button onclick="presetFreq(3500000)">80m</button>
      <button onclick="presetFreq(7000000)">40m</button>
      <button onclick="presetFreq(10100000)">30m</button>
      <button onclick="presetFreq(14000000)">20m</button>
      <button onclick="presetFreq(18000000)">17m</button>
      <button onclick="presetFreq(21000000)">15m</button>
      <button onclick="presetFreq(24890000)">12m</button>
      <button onclick="presetFreq(28000000)">10m</button>
      <button onclick="presetFreq(50000000)">6m</button>
      <button onclick="presetFreq(144000000)">2m</button>
      <button onclick="presetFreq(430000000)">70cm</button>
    </div>

<h3>Shortwave Bands</h3>
<div class="presetRow">
  <!-- lower SW -->
  <button onclick="presetFreq(2300000)">120m</button>
  <button onclick="presetFreq(3200000)">90m</button>
  <button onclick="presetFreq(3900000)">75m</button>
  <button onclick="presetFreq(4750000)">60m</button>
  <button onclick="presetFreq(6050000)">49m</button>
  <button onclick="presetFreq(7200000)">41m</button>
  <button onclick="presetFreq(9500000)">31m</button>
  <button onclick="presetFreq(11700000)">25m</button>
  <button onclick="presetFreq(13570000)">22m</button>
  <button onclick="presetFreq(15100000)">19m</button>
  <button onclick="presetFreq(17700000)">16m</button>
  <button onclick="presetFreq(18900000)">15m</button>
  <button onclick="presetFreq(21500000)">13m</button>
  <button onclick="presetFreq(26100000)">11m</button>
  <button onclick="presetFreq(27185000)">CB (27 MHz)</button>
</div>



<script>


let volume = 0.7;
let squelch = 0;
let currentSignal = 0;
let currentFreq = 7000000;
let memories = JSON.parse(localStorage.getItem('memories') || '[]');
while (memories.length < 24) memories.push(null);
let displayOverride = null;
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);
let scanData = new Array(200).fill(0);  // fixed width buffer
let scanning = false;
let preScanMode = null;
let scanIndex = 0;
let scanStartFreq = 0;
let scanStep = 5000;
let scanPaused = false;
let isDragging = false;
let dragMoved = false;
let dragCursorIndex = null;
let dragStartX = 0;
const DRAG_THRESHOLD = 5; // pixels (tune 3 - 8)
let audioEnv = 0;           // smoothed amplitude
let fadeGain = 1.0;         // current gain multiplier (0..1)
let fadeThreshold = 0.002;   // adjustable (≈ noise floor)
let fadeHoldTime = 400;     // ms before fade starts
let fadeDuration = 500;     // ms fade time
let belowSince = null;      // timestamp when signal dropped
let forceAudioOpen = false;
let stepInterval = null;
let baseFreq = null;   // frequency when user starts dragging
let pendingFreq = null;
let freqTimer = null;


// click processor
let dcPrev = 0;           // previous output sample
let dcHold = 0;           // samples left to hold after a click
let DC_THRESH = 0.5;    // jump threshold (0..1). raise if too aggressive
const DC_HOLD_SAMPLES = 8; // ~1 ms at 8 kHz (tune 4–16)
const DC_BLEND = 0.1;     // interpolation weight when suppressing




function initWebSocket() {
  if (websocket && websocket.readyState === 1) return;

  websocket = new WebSocket(gateway);
  websocket.binaryType = "arraybuffer";

  websocket.onmessage = onMessage;

  websocket.onclose = () => {
    setTimeout(initWebSocket, 1000);
  };

  websocket.onerror = () => {
    websocket.close();
  };
}



function sendFreqDebounced(freq, delay = 40) {
  pendingFreq = freq;

  if (freqTimer) return;

  freqTimer = setTimeout(() => {
    fetch('/setfreq?f=' + pendingFreq);
    freqTimer = null;
  }, delay);
}






function startStep(step) {
  stepFreq(step);
  stepInterval = setInterval(() => stepFreq(step), 300);
}
function stopStep() {
  if (stepInterval) {
    clearInterval(stepInterval);
    stepInterval = null;
  }
}




function onLoad() {


  if (window.__onLoadRan) {
    console.warn("onLoad already executed — skipping");
    return;
  }
  window.__onLoadRan = true;


const fadeEl = document.getElementById('fadeSlider');
if (fadeEl) {
  fadeEl.oninput = (e) => {
    fadeThreshold = 0.002 + (e.target.value / 100) * 0.095;
  };
}


  document.getElementById('volSlider').oninput = (e) => {
  volume = e.target.value / 100;
};

document.getElementById('sqlSlider').oninput = (e) => {
  squelch = parseInt(e.target.value, 10);
};





const clickSlider = document.getElementById('clickSlider');

// map: 0 → 0.5  |  100 → 0
function updateClickThreshold(val) {
   DC_THRESH = 0.4 - (val / 100) * 0.4;
}

clickSlider.value = 0;
updateClickThreshold(0);

clickSlider.oninput = (e) => {
  updateClickThreshold(parseInt(e.target.value, 10));
};

const tuneSlider = document.getElementById('tuneSlider');

const DEAD_ZONE = 8;   // slider units (out of ±100)

tuneSlider.addEventListener('mousedown', () => {
  baseFreq = displayOverride || currentFreq;
});

tuneSlider.addEventListener('touchstart', () => {
  baseFreq = displayOverride || currentFreq;
});

tuneSlider.oninput = () => {

  let val = parseInt(tuneSlider.value, 10);

  if (baseFreq === null) return;

  // ===== DEAD ZONE =====
  if (Math.abs(val) < DEAD_ZONE) {

    // snap slider visually
    if (tuneSlider.value !== "0") {
      tuneSlider.value = 0;
    }

    // restore original frequency
    displayOverride = baseFreq;
    document.getElementById('freqDisplay').value = formatFrequency(baseFreq);
    document.getElementById('freqControlInput').value = Math.round(baseFreq);

    fetch('/setfreq?f=' + baseFreq);

    return;
  }

  // ===== NORMAL OPERATION =====

  // normalize [-1 … +1]
  let norm = val / 100;

  // mode-dependent range
  let mode = parseInt(document.getElementById('mode').value, 10);

  let maxOffset =
    (mode === 1 || mode === 4)   // AM or NBFM
      ? 10000
      : 1000;

  let offset = Math.round(norm * maxOffset);

  let newFreq = baseFreq + offset;

  displayOverride = newFreq;
  document.getElementById('freqDisplay').value = formatFrequency(newFreq);
  document.getElementById('freqControlInput').value = Math.round(newFreq);

sendFreqDebounced(newFreq);
};


  initWebSocket();
  renderMemories();
  document.getElementById('audioOffBtn').classList.add('active');
  document.getElementById('recBtn').disabled = true;

const canvas = document.getElementById('spectrum');

canvas.addEventListener('click', (e) => {

  // if this click came from a drag → IGNORE
  if (dragMoved) {
    dragMoved = false;
    return;
  }

const rect = canvas.getBoundingClientRect();
const scaleX = canvas.width / rect.width;
const x = (e.clientX - rect.left) * scaleX;

const index = indexFromCanvasX(x, canvas);

  if (!scanPaused) {
    stopScanAt(index);
  } else {
    resumeScan();
  }
});

// ===== DRAG TUNING =====
canvas.addEventListener('mousedown', (e) => {
 isDragging = true;
dragMoved = false;

const rect = canvas.getBoundingClientRect();
dragStartX = e.clientX - rect.left;


  if (scanning) {
    scanning = false;
    restoreModeAfterScan();
  }

  scanPaused = true;
  forceAudioOpen = true;
  fadeGain = 1.0;
  belowSince = null;

  const btn = document.getElementById('scanBtn');
  if (btn) btn.textContent = "RESUME SCAN";
});

canvas.addEventListener('mousemove', (e) => {

  if (!isDragging) return;

  const rect = canvas.getBoundingClientRect();

  // --- drag threshold ---
  const dx = (e.clientX - rect.left) - dragStartX;
  if (!dragMoved && Math.abs(dx) < DRAG_THRESHOLD) return;

  dragMoved = true;

  // --- FIXED coordinate scaling ---
  const scaleX = canvas.width / rect.width;
  let x = (e.clientX - rect.left) * scaleX;

  if (x < 0) x = 0;
  if (x > canvas.width) x = canvas.width;

  // --- unified mapping ---
  let index = indexFromCanvasX(x, canvas);
  let freq = scanStartFreq + index * scanStep;

  freq = snap5kHz(freq);

  index = Math.round((freq - scanStartFreq) / scanStep);

  if (index < 0) index = 0;
  if (index >= scanData.length) index = scanData.length - 1;

  dragCursorIndex = index;

  drawSpectrum();

  displayOverride = freq;
  document.getElementById('freqDisplay').value = formatFrequency(freq);
  document.getElementById('freqControlInput').value = Math.round(freq);
});



canvas.addEventListener('mouseup', (e) => {
  if (!isDragging) return;
  isDragging = false;
  forceAudioOpen = false;
belowSince = null;
fadeGain = 1.0;
audioEnv = fadeThreshold * 2;

const rect = canvas.getBoundingClientRect();
const scaleX = canvas.width / rect.width;
const x = (e.clientX - rect.left) * scaleX;

  if (x < 0) x = 0;
  if (x > canvas.width) x = canvas.width;

let index = indexFromCanvasX(x, canvas);
let freq = scanStartFreq + index * scanStep;
freq = snap5kHz(freq);

  // commit final frequency
  sendFreqDebounced(Math.round(freq));

 dragCursorIndex = null;
updateScanCursorFromFreq(freq);

  displayOverride = freq;
  if (!audioRunning) {
     startAudio();
     fetch('/audioctl?on=1');
   }

});



  const el = document.getElementById('freqDisplay');
  if (!el) return;

  // Wheel on frequency display → fine control
  el.addEventListener('wheel', freqWheel);

  // Wheel anywhere else → kHz tuning
  document.addEventListener('wheel', function(event) {
    // ignore if over freqDisplay
    if (event.target === el) return;

    // also ignore if inside it (safety)
    if (el.contains(event.target)) return;

    event.preventDefault();

    let step = 1000;
    if (event.deltaY > 0) step = -step;

    stepFreq(step);
  }, { passive: false });

document.addEventListener("visibilitychange", () => {
  if (!document.hidden) {
    memories = JSON.parse(localStorage.getItem('memories') || '[]');
    while (memories.length < 24) memories.push(null);

    renderMemories();

    // optional: reconnect websocket if needed
    if (!websocket || websocket.readyState !== 1) {
      initWebSocket();
    }
  }
});

} // end onload()



function setTuneFrequency(freq) {
  displayOverride = freq;

  document.getElementById('freqDisplay').value = formatFrequency(freq);
  document.getElementById('freqControlInput').value = Math.round(freq);

  fetch('/setfreq?f=' + freq);
}




function restoreModeAfterScan() {
  if (preScanMode !== null) {
    document.getElementById('mode').value = preScanMode;
    setMode();
    preScanMode = null;
  }
}


function indexFromCanvasX(x, canvas) {
  let ratio = x / canvas.width;
  let index = Math.round(ratio * (scanData.length - 1));

  if (index < 0) index = 0;
  if (index >= scanData.length) index = scanData.length - 1;

  return index;
}




async function startScan() {


  const btn = document.querySelector('button[onclick="startScan()"]');




  // toggle behavior
// toggle behavior
if (scanning) {
  scanning = false;
  restoreModeAfterScan();
  if (btn) btn.textContent = "SCAN";
  return;
}


scanning = true;
scanPaused = false;
fetch('/audioctl?on=0');

// store mode ONLY once
if (preScanMode === null) {
  preScanMode = document.getElementById('mode').value;
}


// force AM
document.getElementById('mode').value = "1";
setMode();

  if (btn) btn.textContent = "STOP SCAN";

  let center = displayOverride || currentFreq;
  scanStartFreq = Math.floor(center / 1000000) * 1000000;


 stopAudio();


forceAudioOpen = true;
fadeGain = 1.0;
belowSince = null;



  while (scanning) {

    let f = scanStartFreq + scanIndex * scanStep;

    displayOverride = f;
document.getElementById('freqDisplay').value = formatFrequency(f);
document.getElementById('freqControlInput').value = Math.round(f);

   sendFreqDebounced(f);
   await sleep(30);

    // wait for fresh reading (with fallback)
    let old = currentSignal;
    let timeout = 0;

    await fetch('/setfreq?f=' + f);
    await sleep(30);   // deterministic pacing
    scanData[scanIndex] = currentSignal;

    // store value
    scanData[scanIndex] = currentSignal;

    drawSpectrum();

    scanIndex++;

    // wrap around (overwrite old line)
    if (scanIndex >= scanData.length) {
      scanIndex = 0;
    }
  }

fetch('/audioctl?on=1');
 startAudio();
}



function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}



function drawSpectrum() {
  const canvas = document.getElementById('spectrum');
  const ctx = canvas.getContext('2d');

  const w = canvas.width;
  const h = canvas.height;

  ctx.fillStyle = "#000";
  ctx.fillRect(0, 0, w, h);

  let stepX = w / scanData.length;

  // ===== GRID: 100 kHz =====
  const binsPer100k = 100000 / scanStep;   // = 20
  ctx.strokeStyle = "#666";                // dark grey
  ctx.lineWidth = 1;

  ctx.beginPath();
  for (let i = 0; i < scanData.length; i += binsPer100k) {
    let x = i * stepX;
    ctx.moveTo(x, 0);
    ctx.lineTo(x, h);
  }
  ctx.stroke();


// ===== HORIZONTAL GRID (5 divisions) =====
ctx.strokeStyle = "#666";
ctx.lineWidth = 1;

ctx.beginPath();

const hDivs = 5;

for (let i = 1; i <= hDivs; i++) {
  let y = (h / (hDivs + 1)) * i;  // evenly spaced
  ctx.moveTo(0, y);
  ctx.lineTo(w, y);
}

ctx.stroke();

  // ===== SCALE (top) =====
ctx.fillStyle = "#fff";
ctx.font = "12px Consolas";

// stronger glow for readability
ctx.shadowColor = "#fff";
ctx.shadowBlur = 6;
ctx.textAlign = "center";



  for (let i = 0; i < scanData.length; i += binsPer100k) {
    let x = i * stepX;

    let freq = scanStartFreq + i * scanStep;

    let label;
    if (freq >= 1e6) {
      label = (freq / 1e6).toFixed(1);   // MHz
    } else {
      label = Math.round(freq / 1000);   // kHz
    }

    ctx.fillText(label, x, 10);
  }
   ctx.shadowBlur = 0;


  // ===== SPECTRUM TRACE =====
  ctx.strokeStyle = "#0f0";
  ctx.lineWidth = 2;
  ctx.beginPath();

  for (let i = 0; i < scanData.length; i++) {

const hDivs = 5;
const yOffset = h / (hDivs + 1);   // one division down

let val = (scanData[i] / 128) * 1.5;  // amplify

// optional clamp (prevents drawing outside canvas)
if (val > 1) val = 1;

let x = i * stepX;
let y = h - (val * h) + yOffset;



    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }

  ctx.stroke();

  // ===== SCAN CURSOR =====
  ctx.strokeStyle = "#ff0";
  ctx.beginPath();
  let cursor = (dragCursorIndex !== null) ? dragCursorIndex : scanIndex;
  let x = (cursor + 0.5) * stepX;  // center of bin
  ctx.moveTo(x, 0);
  ctx.lineTo(x, h);
  ctx.stroke();
}




function freqFromCanvasX(x, canvas) {
  const ratio = x / canvas.width;
  const index = ratio * scanData.length;
  return scanStartFreq + index * scanStep;
}


function snap5kHz(freq) {
  return Math.floor(freq / 5000) * 5000;
}




function updateScanCursorFromFreq(freq) {
  if (!scanStartFreq || !scanStep) return;

  let index = Math.round((freq - scanStartFreq) / scanStep);

  // clamp into visible range
  if (index < 0) index = 0;
  if (index >= scanData.length) index = scanData.length - 1;

  scanIndex = index;

  drawSpectrum();
}



async function stopScanAt(index) {
  scanning = false;
  scanPaused = true;
  restoreModeAfterScan();
  scanIndex = index;
  forceAudioOpen = false;
  belowSince = null;
  fadeGain = 1.0;


let freq = scanStartFreq + index * scanStep;
freq = snap5kHz(freq);

// ensure cursor matches snapped freq
index = Math.round((freq - scanStartFreq) / scanStep);
scanIndex = index;

  displayOverride = freq;
  document.getElementById('freqDisplay').value = formatFrequency(freq);
  document.getElementById('freqControlInput').value = freq;

  fetch('/setfreq?f=' + freq);

  const btn = document.getElementById('scanBtn');
  if (btn) btn.textContent = "RESUME SCAN";

fetch('/audioctl?on=1');
 startAudio();

  drawSpectrum();
}

function resumeScan() {
  forceAudioOpen = false;
  belowSince = null;
  fadeGain = 1.0;


  scanPaused = false;

  const btn = document.getElementById('scanBtn');
  if (btn) btn.textContent = "STOP SCAN";
  stopAudio();
  startScan();
}

function renderMemories() {
  const row = document.getElementById('memoryRow');
  row.innerHTML = '';

  for (let i = 0; i < 24; i++) {
    const btn = document.createElement('button');

    let m = memories[i];

    if (m) {
      // --- label ---
      if (m.freq >= 1000000) {
        btn.textContent = (m.freq / 1000000).toFixed(3) + "M";
      } else {
        btn.textContent = (m.freq / 1000).toFixed(0) + "K";
      }

      // --- color by mode ---
      let color;
      switch (parseInt(m.mode, 10)) {
        case 1: color = "#2a5f2a"; break; // AM → green
        case 2: // LSB
        case 3: // USB
        case 6: // SYNC
        case 7: color = "#2a3f5f"; break; // blue
        case 4: color = "#4b2a5f"; break; // NBFM → purple
        default: color = "#5f5a2a"; break; // others → yellow
      }

      btn.style.background = `linear-gradient(${color}, #111)`;
      btn.style.color = "#eee";
      btn.style.borderColor = "#666";

    } else {
      // --- empty slot ---
      btn.textContent = "M" + (i + 1);
      btn.style.background = "#000";
      btn.style.color = "#555";
      btn.style.borderColor = "#333";
    }

    // --- CLICK = LOAD ---
    btn.onclick = () => {
      if (!memories[i]) return;

      let m = memories[i];

      displayOverride = m.freq;
      document.getElementById('freqDisplay').value = formatFrequency(m.freq);
      document.getElementById('freqControlInput').value = m.freq;
      fetch('/setfreq?f=' + m.freq);
      updateScanCursorFromFreq(m.freq);

      document.getElementById('mode').value = m.mode;
      setMode();
    };

    // --- RIGHT CLICK = SAVE ---
    btn.oncontextmenu = (e) => {
      e.preventDefault();
      saveMemory(i);
    };

    // --- LONG PRESS (mobile) ---
    let pressTimer = null;

    btn.addEventListener('touchstart', () => {
      pressTimer = setTimeout(() => {
        saveMemory(i);
      }, 600);
    });

    btn.addEventListener('touchend', () => {
      clearTimeout(pressTimer);
    });

    btn.addEventListener('touchmove', () => {
      clearTimeout(pressTimer);
    });

    row.appendChild(btn);
  }
}



function saveMemory(index) {
    let freq = displayOverride !== null ? displayOverride : currentFreq;
     if (!freq) return;

  let mode = document.getElementById('mode').value;

  memories[index] = { freq: freq, mode: mode };
  localStorage.setItem('memories', JSON.stringify(memories));

  renderMemories();
}


function getModeColor(mode) {
  switch (parseInt(mode, 10)) {
    case 1: return "#2a5f2a";   // AM → soft green
    case 2: // LSB
    case 3: // USB
    case 6: // SYNC
    case 7: return "#2a3f5f";   // blue group
    case 4: return "#4b2a5f";   // NBFM → soft purple
    case 5: return "#5f5a2a";   // WBFM → yellow-ish
    default: return "#333";     // fallback
  }
}




function onMessage(event) {

  if (!event || !event.data) return;
  // Distinguish between text (control/status) and binary (audio)
  if (typeof event.data === "string") {
    let parts = event.data.split(",");
    if (parts.length == 2) {
      let freq = parseInt(parts[0], 10);
      currentFreq = freq;
      let smeter = parseInt(parts[1]);
      currentSignal = smeter;


if (!isDragging) {
  displayOverride = null;
  document.getElementById('freqDisplay').value = formatFrequency(freq);  // freq change on server
}


      // Scale signal value to 300px bar length
      let maxValue = 128;
      let ratio = smeter / maxValue;
      if (ratio > 1) ratio = 1;

     let barWidth = ratio * document.getElementById('smeter').clientWidth;
      document.getElementById('bar').style.width = barWidth + 'px';

      // color: red → yellow → green based on level
      let r, g;
      if (ratio < 0.5) {
        r = 255;
        g = Math.round(ratio * 2 * 255);
      } else {
        g = 255;
        r = Math.round((1 - (ratio - 0.5) * 2) * 255);
      }
      document.getElementById('bar').style.backgroundColor = `rgb(${r},${g},0)`;
    }
  } else {
    // Binary data = audio samples
    let data = new Uint8Array(event.data);

   for (let i = 0; i < data.length; i++) {
  fifo.push(data[i]);
}

// --- overflow control (based on unread data) ---
let available = fifo.length - fifoRead;


if ((fifo.length - fifoRead) < 200) {
  console.log("UNDERRUN");
}


if (available > 16000) {
  // drop oldest unread samples, keep newest ~8000
  fifoRead = fifo.length - 8000;
}



  }
}


function setFreq() {
  let val = document.getElementById('freqControlInput').value;
  if (!val) return;

  let cleaned = val.replace(/[^0-9.]/g, '');
  let num = parseFloat(cleaned);
  if (isNaN(num)) return;

  let freqHz;
  if (num > 1000000) {
    freqHz = Math.round(num);          // already Hz
  } else {
    freqHz = Math.round(num * 1000);   // convert kHz → Hz
  }

  // normalize display back to kHz
  document.getElementById('freqControlInput').value = Math.round(freqHz / 1000);
  fetch('/setfreq?f=' + freqHz)
    .then(r => r.text())
    .then(v => {
  displayOverride = freqHz;
  document.getElementById('freqDisplay').value = formatFrequency(freqHz);
  updateScanCursorFromFreq(freqHz);
});

}

function formatFrequency(freqHz) {
  let freq = parseInt(freqHz, 10);
  if (freq >= 1000000) {
    let mhz = Math.floor(freq / 1000000);
    let khz = Math.floor((freq % 1000000) / 1000);
    let hz  = freq % 1000;
    return (
      mhz + "." +
      khz.toString().padStart(3, "0") + "." +
      hz.toString().padStart(3, "0") +
      " MHz"
    );
  } else if (freq >= 1000) {
    let khz = Math.floor(freq / 1000);
    let hz  = freq % 1000;
    return (
      khz + "." +
      hz.toString().padStart(3, "0") +
      " KHz"
    );
  } else {
    return freq + " Hz";
  }
}

function freqWheel(event) {
  event.preventDefault();

  const el = event.target;
  const rect = el.getBoundingClientRect();

  let x = event.clientX - rect.left;
  let ratio = x / rect.width;

  if (ratio < 0) ratio = 0;
  if (ratio > 1) ratio = 1;

  let step;

  // tuned boundaries (shift everything left)
  if (ratio < 0.30) {
    step = 1000000;      // MHz
  } else if (ratio < 0.60) {
    step = 1000;         // kHz
  } else if (ratio < 0.80) {
    step = 100;          // 100 Hz
  } else {
    step = 10;           // 10 Hz (far right)
  }

  if (event.deltaY > 0) step = -step;

  stepFreq(step);
}


function stepFreq(step) {
  fetch('/stepfreq?step='+step)
    .then(r=>r.text())
   .then(v=>{
  displayOverride = parseInt(v, 10);
  updateScanCursorFromFreq(displayOverride);
  document.getElementById('freqDisplay').value = formatFrequency(displayOverride);
  document.getElementById('freqControlInput').value = displayOverride;
});
}


function setMode() {
  let modeVal = document.getElementById('mode').value;
  fetch('/setmode?m='+modeVal)
    .then(r=>r.text())
    .then(v=>{ console.log("Mode set to "+modeVal); });
}

function presetFreq(freq) {
  displayOverride = freq;  // trust what we send, not what comes back

  document.getElementById('freqDisplay').value = formatFrequency(freq);
  document.getElementById('freqControlInput').value = freq;

  fetch('/setfreq?f=' + freq);
  updateScanCursorFromFreq(freq);
}



// ===== AUDIO STREAM (smoothed + gain) =====
let audioCtx = null;
let processor = null;
let audioRunning = false;
let fifo = [];
let fifoRead = 0;

let recordBuffer = [];
let isRecording = false;

let lastSample = 0;
let started = false;

const TARGET_BUFFER = 1024;   // startup buffer
const AUDIO_GAIN = 2.0;       // slight increase

function startAudio() {
  if (audioRunning) return;
  audioRunning = true;
  fetch('/audioctl?on=1');

const recBtn = document.getElementById('recBtn');
if (recBtn) recBtn.disabled = false;

  // --- UI ---
  const onBtn = document.getElementById('audioOnBtn');
  const offBtn = document.getElementById('audioOffBtn');

  if (onBtn && offBtn) {
    onBtn.classList.add('active');
    offBtn.classList.remove('active');
  }

  // --- backend ---
  fetch('/audioctl?on=1');

  // --- audio context ---
  audioCtx = new (window.AudioContext || window.webkitAudioContext)({
    sampleRate: 8000
  });

  processor = audioCtx.createScriptProcessor(1024, 1, 1);

  started = false;
  lastSample = 0;
//-----------------------------------------------------------------------//





processor.onaudioprocess = function(e) {
  try {
    let out = e.outputBuffer.getChannelData(0);

    // --- startup buffer (click prevention) ---
    if (!started) {
      if ((fifo.length - fifoRead) > TARGET_BUFFER) {
        started = true;
      } else {
        for (let i = 0; i < out.length; i++) out[i] = 0;
        return;
      }
    }

    const now = (performance && performance.now) ? performance.now() : Date.now();

    for (let i = 0; i < out.length; i++) {

      // ===== INPUT SAMPLE =====
      if (fifoRead < fifo.length) {
        let raw = fifo[fifoRead++];
        if (raw === undefined) raw = 128;
        let sample = (raw - 128) / 128;

        // click smoothing
        if (Math.abs(sample - (lastSample || 0)) > 0.3) {
          lastSample = (lastSample || 0) * 0.8 + sample * 0.2;
        } else {
          lastSample = sample;
        }
      } else {
        // graceful underrun handling
        if (lastSample !== 0) {
          lastSample *= 0.99;
        }
      }

      let sampleOut = lastSample || 0;

      // ===== ENVELOPE =====
      let absSample = Math.abs(sampleOut);
      audioEnv = (audioEnv || 0) * 0.98 + absSample * 0.02;

      // ===== FADE LOGIC =====
      if (forceAudioOpen) {
        fadeGain = 1.0;
        belowSince = null;
      } else {
        let threshold = fadeThreshold || 0.002;
        if (audioEnv < threshold) {
          if (belowSince === null) belowSince = now;
          let dt = now - belowSince;
          if (dt > (fadeHoldTime || 500)) {
            let fadeProgress = (dt - fadeHoldTime) / (fadeDuration || 500);
            fadeProgress = Math.max(0, Math.min(1, fadeProgress));
            fadeGain = 1.0 - fadeProgress;

          }
        } else {
          belowSince = null;
          fadeGain += (1.0 - fadeGain) * 0.1;
        }
      }

      // ===== APPLY =====
      if ((currentSignal || 0) < (squelch || 0)) {
        sampleOut = 0;
      }

      let gain = (volume || 0) * (fadeGain || 1);
      sampleOut *= gain;

      // ===== DE-CLICK FILTER =====
      let diff = Math.abs(sampleOut - dcPrev);
      if (diff > DC_THRESH) {
        dcHold = DC_HOLD_SAMPLES;
      }
      if (dcHold > 0) {

        sampleOut = dcPrev * (1 - DC_BLEND) + sampleOut * DC_BLEND;
        dcHold--;
      }
      dcPrev = sampleOut;

      // ===== OUTPUT =====
      out[i] = Math.tanh(sampleOut * AUDIO_GAIN);

      if (isRecording && recordBuffer) {
        recordBuffer.push(out[i]);
      }
    }

    // ===== FIFO LIMIT =====
    if (fifoRead > 4096) {
      fifo = fifo.slice(fifoRead);
      fifoRead = 0;
    }

  } catch (err) {
    console.error("Audio loop error:", err);
  }
};

//----------------------------------------------------------------------//
  processor.connect(audioCtx.destination);
}


function stopAudio() {
  audioRunning = false;
  started = false;


// reset recording state
isRecording = false;
const recBtnEl = document.getElementById('recBtn');
if (recBtnEl) {
  recBtnEl.classList.remove('active');
  recBtnEl.textContent = "REC";
}


  const onBtn = document.getElementById('audioOnBtn');
  const offBtn = document.getElementById('audioOffBtn');

  const recBtn = document.getElementById('recBtn');
  if (recBtn) recBtn.disabled = true;

  isRecording = false;
  if (onBtn && offBtn) {
    offBtn.classList.add('active');
    onBtn.classList.remove('active');
  }

  fetch('/audioctl?on=0');

  if (processor) {
    processor.disconnect();
    processor = null;
  }

  if (audioCtx) {
    audioCtx.close();
    audioCtx = null;
  }

  fifo = [];
fifoRead = 0;
  lastSample = 0;
}




function encodeWAV(samples, sampleRate) {
  let buffer = new ArrayBuffer(44 + samples.length * 2);
  let view = new DataView(buffer);

  function writeString(view, offset, string) {
    for (let i = 0; i < string.length; i++) {
      view.setUint8(offset + i, string.charCodeAt(i));
    }
  }

  writeString(view, 0, 'RIFF');
  view.setUint32(4, 36 + samples.length * 2, true);
  writeString(view, 8, 'WAVE');

  writeString(view, 12, 'fmt ');
  view.setUint32(16, 16, true);
  view.setUint16(20, 1, true); // PCM
  view.setUint16(22, 1, true); // mono
  view.setUint32(24, sampleRate, true);
  view.setUint32(28, sampleRate * 2, true);
  view.setUint16(32, 2, true);
  view.setUint16(34, 16, true);

  writeString(view, 36, 'data');
  view.setUint32(40, samples.length * 2, true);

  let offset = 44;
  for (let i = 0; i < samples.length; i++, offset += 2) {
    let s = Math.max(-1, Math.min(1, samples[i]));
    view.setInt16(offset, s * 32767, true);
  }

  return buffer;
}


function toggleRecording() {
  const btn = document.getElementById('recBtn');

  if (!audioRunning) return; // safety

  if (!isRecording) {
    // START recording
    recordBuffer = [];
    isRecording = true;

    btn.classList.add('active');
    btn.textContent = "STOP";

  } else {
    // STOP recording
    isRecording = false;

    btn.classList.remove('active');
    btn.textContent = "REC";

    // --- get frequency ---
    let freq = displayOverride;

    if (!freq) {
      let txt = document.getElementById('freqDisplay').value || "";
      freq = txt.replace(/[^0-9]/g, '');
    }

    // --- get mode text ---
    let modeSelect = document.getElementById('mode');
    let modeText = modeSelect.options[modeSelect.selectedIndex].text;

    // --- format frequency ---
    let freqStr = "unknown";
    if (freq) {
      freq = parseInt(freq, 10);
      if (freq >= 1000000) {
        freqStr = (freq / 1000000).toFixed(3) + "MHz";
      } else {
        freqStr = Math.round(freq / 1000) + "kHz";
      }
    }

    // --- sanitize mode ---
    let modeStr = modeText.replace(/[^a-zA-Z0-9]/g, '');

    // --- timestamp ---
    let now = new Date();
    let timeStr =
      now.getFullYear() +
      String(now.getMonth() + 1).padStart(2, '0') +
      String(now.getDate()).padStart(2, '0') + "_" +
      String(now.getHours()).padStart(2, '0') +
      String(now.getMinutes()).padStart(2, '0') +
      String(now.getSeconds()).padStart(2, '0');

    // --- filename ---
    let filename = `${freqStr}_${modeStr}_${timeStr}.wav`;

    // --- create WAV ---
    let wav = encodeWAV(recordBuffer, 8000);
    let blob = new Blob([wav], { type: 'audio/wav' });

    let url = URL.createObjectURL(blob);

    let a = document.createElement('a');
    a.href = url;
    a.download = filename;
    a.click();
  }
}


</script>
</div>
</body>
</html>
)rawliteral";

