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

    if (msg.startsWith("LED"))
    {
        if (msg.endsWith("_ON"))
        {
            ledState = true;
            newCommand = true;
            // Serial.println("👉 Received LED_ON");
        }
        else if (msg.endsWith("_OFF"))
        {
            ledState = false;
            newCommand = true;
            // Serial.println("👉 Received LED_OFF");
        }
    }
}

void TaskGPIO(void *pvParameters)
{
    for (;;)
    {
        if (newCommand)
        {
            if (ledState)
            {
                pixels.setPixelColor(0, pixels.Color(255, 0, 255)); // tím
            }
            else
            {
                pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // tắt
            }
            pixels.show();
            writeLedstate();
            newCommand = false;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
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
