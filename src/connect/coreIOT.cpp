#include "coreIOT.h"

const char *mqtt_server = "app.coreiot.io";
const int mqtt_port = 1883;
const char *clientId = "Assignment ESys";
const char *username = "w6vg9vsbydh5liar0yyj";
const char *passID = "";

WiFiClient espClient;
PubSubClient client(espClient);

// Mutex bảo vệ dữ liệu
SemaphoreHandle_t soilMutex;
SemaphoreHandle_t tempMutex;
SemaphoreHandle_t humiMutex;
unsigned long lastSend = 0;
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (int i = 0; i < length; i++)
        msg += (char)payload[i];

    Serial.printf("📥 [%s]: %s\n", topic, msg.c_str());

    if (String(topic).startsWith("v1/devices/me/rpc/request/"))
    {
        // Extract request ID
        String requestId = String(topic).substring(26);

        // Example RPC: {"method":"setValue","params":42}
        StaticJsonDocument<256> doc;
        deserializeJson(doc, msg);

        String method = doc["method"];
        int value = doc["params"];

        // Send RPC response (TB requires this)
        String response = "{\"result\":1}";
        String respTopic = "v1/devices/me/rpc/response/" + requestId;

        client.publish(respTopic.c_str(), response.c_str(), true);
    }
}

// Hàm tiện ích đọc an toàn
float safeRead(float *var, SemaphoreHandle_t mutex)
{
    float value;
    if (mutex != NULL)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        value = *var;
        xSemaphoreGive(mutex);
    }
    else
    {
        value = *var;
    }
    return value;
}

void sendTelemetry(float temperature, float humidity, float soil)
{
    String payload = "{\"temperature\":" + String(temperature, 1) +
                     ",\"humidity\":" + String(humidity, 1) +
                     ",\"soil\":" + String(soil, 1) + "}";

    Serial.print("📤 Sending payload: ");
    Serial.println(payload);
    client.publish("v1/devices/me/telemetry", payload.c_str(), true);
}
void reconnectMQTT()
{
    while (!client.connected())
    {
        Serial.print("➡️ Connecting to Core IOT... ");

        if (client.connect(clientId, username, passID))
        {
            Serial.println("✅ Connected!");
            client.subscribe("v1/devices/me/rpc/request/+");
            client.subscribe("v1/devices/me/attributes");
        }
        else
        {
            Serial.println("❌ MQTT connect fail, retrying...");
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}

void sendDatatoCore(void *pvParameters)
{
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);

    for (;;)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            if (!client.connected())
                reconnectMQTT();

            client.loop();

            if (millis() - lastSend > 60000) // gửi mỗi 60s
            {
                float tempValue = safeRead(&temperature, tempMutex);
                float humiValue = safeRead(&humidity, humiMutex);
                float soilValue = safeRead(&sensorSoilValue, soilMutex);

                sendTelemetry(tempValue, humiValue, soilValue);
                lastSend = millis();
            }
        }
        else
        {
            // Serial.println("⚠️ WiFi chưa sẵn sàng → MQTT tạm ngưng");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // kiểm tra 1s 1 lần
    }
}

void coreIOT_init()
{
    soilMutex = xSemaphoreCreateMutex();
    tempMutex = xSemaphoreCreateMutex();
    humiMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        sendDatatoCore,
        "sendDatatoCore",
        12288,
        NULL,
        4,
        NULL,
        1);
}
