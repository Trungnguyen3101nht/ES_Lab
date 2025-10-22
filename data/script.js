let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

window.addEventListener('load', onLoad);

function onLoad() {
  initWebSocket();

  // LED toggle
  const ledToggle = document.getElementById('ledToggle');
  ledToggle.addEventListener('change', () => {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
      websocket.send(ledToggle.checked ? 'ON' : 'OFF');
      document.getElementById('status').textContent = ledToggle.checked ? 'LED ON' : 'LED OFF';
    }
  });

  // Relay toggles (RELAY1...RELAY4)
  for (let i = 1; i <= 4; i++) {
    const relayToggle = document.getElementById(`relay${i}`);
    const relayStatus = document.getElementById(`relayStatus${i}`);

    relayToggle.addEventListener('change', () => {
      if (websocket && websocket.readyState === WebSocket.OPEN) {
        const command = relayToggle.checked ? `RELAY${i}_ON` : `RELAY${i}_OFF`;
        console.log(`Sending command: ${command}`);
        websocket.send(command);
        relayStatus.textContent = relayToggle.checked ? `Relay ${i} ON` : `Relay ${i} OFF`;
      }
    });
  }
}

function initWebSocket() {
  console.log('Connecting to WebSocket...');
  websocket = new WebSocket(gateway);

  websocket.onopen = () => console.log('✅ Connected to WebSocket');

  websocket.onclose = () => {
    console.log('❌ WebSocket closed, retrying...');
    setTimeout(initWebSocket, 2000);
  };

  websocket.onmessage = (event) => {
    console.log('📩 Message from ESP32:', event.data);
    try {
      const data = JSON.parse(event.data);

      // Cập nhật nhiệt độ / độ ẩm
      if ('temperature' in data && 'humidity' in data) {
        document.getElementById("temp").textContent = data.temperature.toFixed(2);
        document.getElementById("humi").textContent = data.humidity.toFixed(2);
      }

      // Cập nhật trạng thái relay
      if ('relays' in data) {
        data.relays.forEach((state, index) => {
          const toggle = document.getElementById(`relay${index + 1}`);
          const status = document.getElementById(`relayStatus${index + 1}`);
          if (toggle) {
            toggle.checked = state;
            status.textContent = state ? `Relay ${index + 1} ON` : `Relay ${index + 1} OFF`;
          }
        });
      }

    } catch (err) {
      console.warn("⚠️ Invalid JSON:", event.data);
    }
  };
}
