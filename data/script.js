// ==========================
// script.js
// ==========================

const gateway = `ws://${window.location.hostname}/ws`;
let ws;

// Khi load trang
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
    // 1️⃣ Relay toggles
    for (let i = 1; i <= 2; i++) {
        const relayToggle = document.getElementById(`relay${i}`);
        relayToggle.onchange = () =>
            sendCommand(relayToggle.checked ? `RELAY${i}_ON` : `RELAY${i}_OFF`);
    }

    // 2️⃣ Toggle ngày tuần
    document.querySelectorAll(".days-box input[type=checkbox]").forEach(cb => {
    cb.addEventListener("change", () => {
        cb.parentElement.classList.toggle("active", cb.checked);
    });
});

    // 3️⃣ Nút Thêm lịch
    const addBtn = document.getElementById("addScheduleBtn");
    addBtn.onclick = () => {
        const days = [];
        document.querySelectorAll(".days-box input[type=checkbox]:checked")
            .forEach(cb => days.push(parseInt(cb.value)));

        const msg = {
            schedule: {
                relay: parseInt(document.getElementById("schRelay").value)-1,
                state: document.getElementById("schState").value === "true",
                hour: parseInt(document.getElementById("schHour").value),
                minute: parseInt(document.getElementById("schMinute").value),
                days: days
            }
        };

        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify(msg));
            console.log("📤 Thêm lịch:", msg);
        } else {
            console.warn("⚠️ WebSocket chưa sẵn sàng, lịch chưa gửi được");
        }
    };
}



// Gửi lệnh đơn giản qua WebSocket
function sendCommand(cmd) {
    if (ws?.readyState === WebSocket.OPEN) {
        console.log(`📤 Sending: ${cmd}`);
        ws.send(cmd);
    } else {
        console.warn("⚠️ WebSocket not ready, command skipped");
    }
}

function handleScheduleList(data) {
    const list = document.getElementById("scheduleList");
    if(!list) return;

    list.innerHTML = "";

    data.schedules.forEach((s, i) => {
        const li = document.createElement("li");
        li.textContent = `Relay ${s.relay} → ${s.state ? "ON" : "OFF"} lúc ${s.hour}:${s.minute.toString().padStart(2,'0')} ngày ${s.days.join(",")}`;

        const delBtn = document.createElement("button");
        delBtn.textContent = "Xóa";
        delBtn.onclick = () => {
            ws.send(JSON.stringify({ deleteSchedule: i }));
            li.remove(); 
        };


        li.appendChild(delBtn);
        list.appendChild(li);
    });
}

// Xử lý message từ ESP32
function handleMessage(raw) {
    try {
        const data = JSON.parse(raw);

        // 🌡️ Cập nhật nhiệt độ & độ ẩm
        if ("temperature" in data && "humidity" in data) {
            document.getElementById("temp").textContent = data.temperature.toFixed(2);
            document.getElementById("humi").textContent = data.humidity.toFixed(2);
        }

        // 🌱 Cập nhật soil
        if ("soil" in data) {
            document.getElementById("soil").textContent = data.soil.toFixed(2);
        }

        // ⚙️ Cập nhật relay state
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

        // 📅 Cập nhật danh sách lịch
        if ("schedules" in data) {
            handleScheduleList(data);
        }

    } catch(e) {
        console.warn("⚠️ Invalid JSON:", raw);
    }
}
