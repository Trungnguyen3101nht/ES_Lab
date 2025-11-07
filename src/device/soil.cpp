#include "soil.h"
float sensorValue = 0;
#define SOIL_PIN 2 // hoặc 34, 32... tùy chân ADC bạn dùng
void readSoil()
{
    int maxSoilValue = 4095; // giá trị ướt nhất (thay đổi tùy theo cảm biến và mạch)
    int minSoilValue = 0;    // giá trị khô nhất (thay đổi tùy theo cảm biến và mạch)
    float SoilValueraw = analogRead(SOIL_PIN);
    int soilPercent = (SoilValueraw / maxSoilValue) * 100;
    // float SoilValue = analogRead(SOIL_PIN);
    if (soilPercent >= 0)
    {
        sensorValue = soilPercent;
    }
}
void TaskSoil(void *pvParameters)
{
    Serial.println("Starting TaskSoil...");
    while (true)
    {
        readSoil();

        if (WiFi.status() == WL_CONNECTED)
        {
            if (ws.count() > 0)
            {
                String data = "{\"soil\":" + String(sensorValue, 2) + "}";
                ws.textAll(data);
                Serial.println(data);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void SoilSensor_Init()
{
    xTaskCreatePinnedToCore(
        TaskSoil,   // hàm task
        "TaskSoil", // tên task
        4096,       // stack size (tăng lên 4096 cho an toàn)
        NULL,       // tham số
        1,          // priority
        NULL,       // task handle
        1);         // core 1
}
