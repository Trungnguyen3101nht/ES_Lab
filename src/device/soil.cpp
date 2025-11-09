#include "soil.h"
float sensorSoilValue = 0;

void readSoil()
{
    int maxSoilValue = 4095; // giá trị ướt nhất (thay đổi tùy theo cảm biến và mạch)
    int minSoilValue = 0;    // giá trị khô nhất (thay đổi tùy theo cảm biến và mạch)
    float SoilValueraw = analogRead(SOIL_PIN);
    float soilPercent = (SoilValueraw / maxSoilValue) * 100;
    // float SoilValue = analogRead(SOIL_PIN);
    if (soilPercent >= 0)
    {
        sensorSoilValue = soilPercent;
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
                String data = "{\"soil\":" + String(sensorSoilValue, 2) + "}";
                ws.textAll(data);
                // Serial.println(data);
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
