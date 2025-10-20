let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

window.addEventListener('load', onLoad);

function onLoad() {
  initWebSocket();
  const toggle = document.getElementById('ledToggle');
  toggle.addEventListener('change', () => {
    if (toggle.checked) {
      websocket.send('ON');
      document.getElementById('status').textContent = 'LED ON';
    } else {
      websocket.send('OFF');
      document.getElementById('status').textContent = 'LED OFF';
    }
  });
}

function initWebSocket() {
  console.log('Connecting to WebSocket...');
  websocket = new WebSocket(gateway);
  websocket.onopen = () => console.log('Connected to WebSocket');
  websocket.onclose = () => {
    console.log('WebSocket closed, retrying...');
    setTimeout(initWebSocket, 2000);
  };
  websocket.onmessage = onMessage;
}

function onMessage(event) {
  console.log('Message from ESP32:', event.data);
}
