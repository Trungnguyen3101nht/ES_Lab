#include "relay.h"

const int NUM_RELAYS = 4;
const int relayPins[NUM_RELAYS] = {Relay_1, Relay_2, Relay_3, Relay_4}; // ví dụ các chân GPIO

bool RelayState[NUM_RELAYS] = {false, false, false, false};
bool newCommandofRelay = false;
int relayCommandIndex = -1;
bool relayCommandState = false;

// gửi trạng thái tất cả relay về client
void writeRelayState()
{
    DynamicJsonDocument doc(256);
    JsonArray arr = doc.createNestedArray("relays");
    for (int i = 0; i < NUM_RELAYS; i++)
    {
        arr.add(RelayState[i]);
    }
    String json;
    serializeJson(doc, json);
    ws.textAll(json);
}

void handleWSMesOfRelay(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        String msg = (char *)data;

        if (msg.startsWith("RELAY"))
        {
            int relayNum = msg.substring(5, 6).toInt() - 1;
            if (relayNum >= 0 && relayNum < NUM_RELAYS)
            {
                bool state = msg.endsWith("_ON");
                RelayState[relayNum] = state;
                relayCommandIndex = relayNum;
                relayCommandState = state;
                newCommandofRelay = true;
            }
        }
    }
}

void TaskRELAY(void *pvParameters)
{
    for (;;)
    {
        if (newCommandofRelay)
        {
            if (relayCommandIndex >= 0 && relayCommandIndex < NUM_RELAYS)
            {
                digitalWrite(relayPins[relayCommandIndex], relayCommandState ? HIGH : LOW);
            }
            writeRelayState();
            newCommandofRelay = false;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Khởi tạo relay
void Relay_Init()
{
    for (int i = 0; i < NUM_RELAYS; i++)
    {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW);
    }

    xTaskCreatePinnedToCore(
        TaskRELAY,
        "TaskRELAY",
        2048,
        NULL,
        1,
        NULL,
        1);
}
