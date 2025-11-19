#include "relay.h"

bool relayState[4] = {false};
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
    String msg = String((char *)data, len);

    if (!msg.startsWith("RELAY"))
        return;

    int index = msg.substring(5, 6).toInt() - 1;
    bool state = msg.endsWith("_ON");

    CommandMsg_t cmd;
    cmd.cmdType = 2;
    cmd.Value01 = index;
    cmd.Value02 = state;

    xQueueSend(commandQueue, &cmd, 0);
}

void TaskRelay(void *pvParameters)
{
    CommandMsg_t cmd;

    for (;;)
    {
        if (xQueueReceive(commandQueue, &cmd, portMAX_DELAY) == pdPASS)
        {
            if (cmd.cmdType == 2) // Relay command
            {
                int idx = (int)cmd.Value01;
                bool state = cmd.Value02 > 0.5; // ép kiểu float → bool

                digitalWrite(relayPins[idx], state ? HIGH : LOW);
                relayState[idx] = state;

                writeRelayState();
            }
        }
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
