#include "ledRGB.h"

bool ledState = false; // biến LED
// commandQueue khai báo ở global, khởi tạo ở main hoặc nơi init FreeRTOS

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
    String msg = String((char *)data, len);
    CommandMsg_t cmd;
    cmd.cmdType = 1; // LED command

    if (msg.endsWith("_ON"))
        cmd.Value01 = 1;
    else if (msg.endsWith("_OFF"))
        cmd.Value01 = 0;
    else
        return;

    cmd.Value02 = 0;
    xQueueSend(commandQueue, &cmd, 0);
}

void TaskGPIO(void *pvParameters)
{
    CommandMsg_t command;

    for (;;)
    {
        // Chờ lệnh mới từ queue
        if (xQueueReceive(commandQueue, &command, portMAX_DELAY) == pdPASS)
        {
            if (command.cmdType == 1) // LED
            {
                ledState = command.Value01 > 0.5 ? true : false;

                if (ledState)
                    pixels.setPixelColor(0, pixels.Color(255, 0, 255));
                else
                    pixels.setPixelColor(0, pixels.Color(0, 0, 0));

                pixels.show();
                writeLedstate();
            }
            // Có thể thêm các loại cmdType khác ở đây
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
