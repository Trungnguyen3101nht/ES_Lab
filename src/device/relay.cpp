#include "relay.h"

bool newCommandRelay = false;
int relayIndex = -1;
bool relayNewState = false;
bool relayState[NUM_RELAYS] = {false};
const int relayPins[NUM_RELAYS] = {Relay_1, Relay_2};

// Gửi trạng thái tất cả relay về client
void writeRelayState()
{
    DynamicJsonDocument doc(256);
    JsonArray arr = doc.createNestedArray("relays");
    for (bool state : relayState)
        arr.add(state);

    String json;
    serializeJson(doc, json);
    ws.textAll(json);
}
#include <ArduinoJson.h>

void handleRelayUpdate(const char *payload)
{
    StaticJsonDocument<128> doc; // hoặc JsonDocument doc; nếu dùng ArduinoJson v7+
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.print("❌ JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Duyệt qua tất cả relay
    for (int i = 0; i < NUM_RELAYS; i++)
    {
        String key = "stateRelay" + String(i + 1);
        if (!doc[key].isNull())
        {
            bool state = doc[key].as<bool>();
            relayState[i] = state;
            digitalWrite(relayPins[i], state ? HIGH : LOW);

            Serial.printf("🔁 Relay %d -> %s\n", i + 1, state ? "ON" : "OFF");
        }
    }

    // Nếu dùng WebSocket, cập nhật lại client
    writeRelayState();
}

// Xử lý lệnh WebSocket
void handleWSMesOfRelay(void *arg, uint8_t *data, size_t len)
{
    String msg = (char *)data;

    if (msg.startsWith("RELAY"))
    {
        int index = msg.substring(5, 6).toInt() - 1; // RELAY1 → index = 0
        if (index >= 0 && index < NUM_RELAYS)
        {
            bool state = msg.endsWith("_ON");
            relayState[index] = state;
            relayIndex = index;
            relayNewState = state;
            newCommandRelay = true;
            sendRelayStateToCore(); // 🟢 Gửi trạng thái ban đầu lên Core IOT

            Serial.printf("👉 Received %s → Relay %d %s\n", msg.c_str(), index + 1, state ? "ON" : "OFF");
        }
    }
}

// Task điều khiển relay
void TaskRelay(void *pvParameters)
{
    for (;;)
    {
        if (newCommandRelay && relayIndex >= 0 && relayIndex < NUM_RELAYS)
        {
            digitalWrite(relayPins[relayIndex], relayNewState ? HIGH : LOW);
            writeRelayState();
            newCommandRelay = false;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Khởi tạo relay
void Relay_Init()
{
    for (int pin : relayPins)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
    sendRelayStateToCore(); // 🟢 Gửi trạng thái ban đầu lên Core IOT

    xTaskCreatePinnedToCore(
        TaskRelay,
        "TaskRelay",
        2048,
        NULL,
        3,
        NULL,
        1);
}
