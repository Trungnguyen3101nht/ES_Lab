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
        int index = msg.substring(5, 6).toInt() - 1;
        bool state = msg.endsWith("_ON");

        CommandMsg_t cmd;
        cmd.cmdType = 2; // 2 = RELAY
        cmd.Value01 = index;
        cmd.Value02 = state;

        xQueueSendToBack(commandQueue, &cmd, 0);
    }
}

void TaskRelay(void *pvParameters)
{

    CommandMsg_t cmd;
    for (;;)
    {
        if (xQueueReceive(commandQueue, &cmd, portMAX_DELAY))
        {
            if (cmd.cmdType == 2) // Relay command
            {
                digitalWrite(relayPins[(int)cmd.Value01],
                             cmd.Value02 ? HIGH : LOW);

                relayState[(int)cmd.Value01] = cmd.Value02;

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
