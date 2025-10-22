#include "tempandhumi.h"

float temperature = 0, humidity = 0;

DHT20 dht20;

void DHT20_sensor()
{
    if (dht20.read() == DHT20_OK)
    {
        temperature = dht20.getTemperature();
        humidity = dht20.getHumidity();
    }
}

void TaskTempAndHumi(void *pvParameters)
{
    Serial.println("Starting TaskTempAndHumi...");
    Wire.begin(MY_SCL, MY_SDA);
    Wire.setClock(100000);
    dht20.begin();

    while (true)
    {
        DHT20_sensor();

        if (WiFi.status() == WL_CONNECTED)
        {
            if (ws.count() > 0)
            {
                String data = "{\"temperature\":" + String(temperature, 2) + ",\"humidity\":" + String(humidity, 2) + "}";
                ws.textAll(data);
                // Serial.println(data);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void TempandHumi_init()
{
    xTaskCreatePinnedToCore(
        TaskTempAndHumi,
        "TaskTempAndHumi",
        4096,
        NULL,
        1,
        NULL,
        1);
}