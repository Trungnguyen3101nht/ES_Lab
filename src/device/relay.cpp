#include "relay.h"

constexpr int NUM_RELAYS = 4;
const int relayPins[NUM_RELAYS] = {Relay_1, Relay_2, Relay_3, Relay_4};

bool relayState[NUM_RELAYS] = {false};
bool newCommandRelay = false;
int relayIndex = -1;
bool relayNewState = false;

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

            Serial.printf("👉 Received %s → Relay %d %s\n",
                          msg.c_str(), index + 1, state ? "ON" : "OFF");
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

    xTaskCreatePinnedToCore(
        TaskRelay,
        "TaskRelay",
        2048,
        NULL,
        1,
        NULL,
        1);
}
