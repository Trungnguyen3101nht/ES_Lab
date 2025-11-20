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
    dht20.begin();

    while (true)
    {
        DHT20_sensor();
        CommandMsg_t msg;
        msg.cmdType = 3; // 3 = SENSOR temp/humi
        msg.Value01 = temperature;
        msg.Value02 = humidity;
        // Serial.printf(msg.cmdType == 3 ? "Temp: %.2f C, Humi: %.2f %%\n" : "", msg.Value01, msg.Value02);
        xQueueSendToBack(commandQueue, &msg, 0);

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