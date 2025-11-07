const gateway = `ws://${window.location.hostname}/ws`;
let ws;

window.addEventListener("load", () => initWebSocket());

function initWebSocket() {
  console.log("🔌 Connecting to WebSocket...");
  ws = new WebSocket(gateway);

  ws.onopen = () => {
    console.log("✅ WebSocket connected");
    setupUI();
  };

  ws.onclose = () => {
    console.warn("❌ WebSocket closed, reconnecting...");
    setTimeout(initWebSocket, 2000);
  };

  ws.onmessage = (event) => handleMessage(event.data);
}

function setupUI() {
  // 🔘 LED toggle
  const ledToggle = document.getElementById("ledToggle");
  ledToggle.onchange = () => sendCommand(ledToggle.checked ? "LED_ON" : "LED_OFF");

  // 🔲 Relay toggles (RELAY1...RELAY4)
  for (let i = 1; i <= 4; i++) {
    const relayToggle = document.getElementById(`relay${i}`);
    relayToggle.onchange = () =>
      sendCommand(relayToggle.checked ? `RELAY${i}_ON` : `RELAY${i}_OFF`);
  }
}

function sendCommand(cmd) {
  if (ws?.readyState === WebSocket.OPEN) {
    console.log(`📤 Sending: ${cmd}`);
    ws.send(cmd);
  } else {
    console.warn("⚠️ WebSocket not ready, command skipped");
  }
}

function handleMessage(raw) {
  try {
    const data = JSON.parse(raw);

    // 🌡️ Update temperature & humidity
    if ("temperature" in data && "humidity" in data) {
      document.getElementById("temp").textContent = data.temperature.toFixed(2);
      document.getElementById("humi").textContent = data.humidity.toFixed(2);
    }
    if ("soil" in data) {
      document.getElementById("soil").textContent = data.soil.toFixed(2);
    }
    // 💡 Update LED state
    if ("led" in data) {
      const ledToggle = document.getElementById("ledToggle");
      const status = document.getElementById("status");
      ledToggle.checked = data.led;
      status.textContent = data.led ? "LED ON" : "LED OFF";
    }

    // ⚙️ Update relay states
    if ("relays" in data) {
      data.relays.forEach((state, i) => {
        const toggle = document.getElementById(`relay${i + 1}`);
        const status = document.getElementById(`relayStatus${i + 1}`);
        if (toggle && status) {
          toggle.checked = state;
          status.textContent = state ? `Relay ${i + 1} ON` : `Relay ${i + 1} OFF`;
        }
      });
    }
  } catch {
    console.warn("⚠️ Invalid JSON:", raw);
  }
}
