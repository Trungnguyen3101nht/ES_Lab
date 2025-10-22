#include "ledRGB.h"

bool ledState = false;
int newCommand = 0;
int ledCommand = 0;

void createLedRGB()
{
    pixels.begin();            // KHỞI TẠO bắt buộc
    pixels.setBrightness(255); // độ sáng 0–255
    pixels.clear();            // xóa tất cả
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
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        String msg = (char *)data;
        if (msg == "toggle")
        {
            ledState = !ledState;
            ledCommand = ledState;
            newCommand = true;
        }
    }
}

void TaskGPIO(void *pvParameters)
{
    for (;;)
    {
        if (newCommand)
        {
            digitalWrite(LED_GPIO, ledCommand ? HIGH : LOW);
            writeLedstate();
            newCommand = false;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // delay nhỏ để tránh busy loop
    }
}
void LedRGB_Init()
{
    xTaskCreatePinnedToCore(
        TaskGPIO,
        "TaskGPIO",
        2048,
        NULL,
        1,
        NULL,
        1);
}