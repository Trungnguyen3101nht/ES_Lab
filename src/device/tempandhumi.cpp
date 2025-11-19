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
        CommandMsg_t msg;
        msg.cmdType = 3; // 3 = SENSOR temp/humi
        msg.Value01 = temperature;
        msg.Value02 = humidity;

        xQueueSendToBack(commandQueue, &msg, 0);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void TaskSensorWebSocket(void *pv)
{
    // Wire.begin(MY_SCL, MY_SDA);
    // Wire.setClock(100000);
    // dht20.begin();

    CommandMsg_t msg;
    msg.cmdType = 3;

    while (true)
    {
        if (dht20.read() == DHT20_OK)
        {
            msg.Value01 = dht20.getTemperature();
            msg.Value02 = dht20.getHumidity();
            xQueueSendToBack(commandQueue, &msg, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TempandHumi_init()
{
    xTaskCreatePinnedToCore(TaskSensorWebSocket,
                            "SensorWS",
                            4096,
                            NULL,
                            2,
                            NULL,
                            1);

    xTaskCreatePinnedToCore(
        TaskTempAndHumi,
        "TaskTempAndHumi",
        4096,
        NULL,
        1,
        NULL,
        1);
}