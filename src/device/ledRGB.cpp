#include "ledRGB.h"

bool ledState = false;
bool newCommand = false;

void createLedRGB()
{
    pixels.begin();
    pixels.setBrightness(255);
    pixels.clear();
    pixels.show();
}

void writeLedstate()
{
    DynamicJsonDocument doc(128);
    doc["led"] = ledState;
    String json;
    serializeJson(doc, json);
    ws.textAll(json);
}

void handleWSMesOfLED(void *arg, uint8_t *data, size_t len)
{
    String msg = (char *)data;
    bool state;

    if (msg.startsWith("LED"))
    {
        if (msg.endsWith("_ON"))
        {
            state = true;
            xQueueSend(commandQueue, &state, 0);
            Serial.println("👉 Received LED_ON");
        }
        else if (msg.endsWith("_OFF"))
        {
            state = false;
            xQueueSend(commandQueue, &state, 0);
            Serial.println("👉 Received LED_OFF");
        }
    }
}

void TaskGPIO(void *pvParameters)
{
    bool command;

    for (;;)
    {
        // Chờ lệnh mới từ queue
        if (xQueueReceive(commandQueue, &command, portMAX_DELAY) == pdPASS)
        {
            ledState = command;

            if (ledState)
            {
                pixels.setPixelColor(0, pixels.Color(255, 0, 255));
            }
            else
            {
                pixels.setPixelColor(0, pixels.Color(0, 0, 0));
            }

            pixels.show();
            writeLedstate();
        }
    }
}

void LedRGB_Init()
{
    createLedRGB();
    xTaskCreatePinnedToCore(
        TaskGPIO,
        "TaskGPIO",
        2048,
        NULL,
        1,
        NULL,
        1);
}
