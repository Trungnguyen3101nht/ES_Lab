#include "coreIOT.h"

const char *mqtt_server = "app.coreiot.io";
const int mqtt_port = 1883;
const char *clientId = "[ESys]";
const char *username = "OJLyZ0902TxctlHBXdhq";
const char *passID = "";

WiFiClient espClient;
PubSubClient client(espClient);

// Mutex bảo vệ dữ liệu
SemaphoreHandle_t soilMutex;
SemaphoreHandle_t tempMutex;
SemaphoreHandle_t humiMutex;
unsigned long lastSend = 0;
String MesUpdate;
String lastJson(const String &msg)
{
    int lastOpen = msg.lastIndexOf('{');
    int lastClose = msg.lastIndexOf('}');

    if (lastOpen >= 0 && lastClose >= 0 && lastClose > lastOpen)
    {
        return msg.substring(lastOpen, lastClose + 1);
    }

    return ""; // không tìm thấy
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{

    for (int i = 0; i < length; i++)
        MesUpdate += (char)payload[i];

    // Serial.printf("📥hhehe [%s]: %s\n", topic, MesUpdate.c_str());
    // Serial.println(MesUpdate.c_str());
    if (String(topic) == "v1/devices/me/attributes")
    {
        String jsonStr = lastJson(MesUpdate); // lấy JSON cuối
        if (jsonStr.length() > 0)
        {
            handleRelayUpdate(jsonStr.c_str());
        }
    }
    if (String(topic).startsWith("v1/devices/me/rpc/request/"))
    {
        // Extract request ID
        String requestId = String(topic).substring(26);

        // Example RPC: {"method":"setValue","params":42}
        // StaticJsonDocument<256> doc;
        JsonDocument doc;

        deserializeJson(doc, MesUpdate);

        String method = doc["method"];
        int value = doc["params"];

        // Send RPC response (TB requires this)
        String response = "{\"result\":1}";
        String respTopic = "v1/devices/me/rpc/response/" + requestId;

        client.publish(respTopic.c_str(), response.c_str(), true);
    }

    if (String(topic) == "v1/devices/me/attributes")
    {
        // StaticJsonDocument<256> doc;
        JsonDocument doc;

        deserializeJson(doc, MesUpdate);

        if (!doc["shared"].isNull())
        {
            JsonObject shared = doc["shared"];
            if (!shared["relays"].isNull())
            {
                JsonArray arr = shared["relays"];
                for (int i = 0; i < arr.size() && i < NUM_RELAYS; i++)
                {
                    relayState[i] = arr[i];
                    digitalWrite(relayPins[i], relayState[i] ? HIGH : LOW);
                }
                Serial.println("🔁 Updated relays from Core IOT");

                // Gửi ngược lại cho WebSocket client
                writeRelayState();
            }
        }
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
void sendRelayStateToCore()
{
    JsonDocument doc;

    for (int i = 0; i < NUM_RELAYS; i++)
    {
        String key = "relay" + String(i + 1);
        doc[key] = relayState[i];
    }

    char payload[256];
    serializeJson(doc, payload);
    client.publish("v1/devices/me/attributes", payload, true);

    // Serial.printf("📤1 Sent relay states to Core IOT: %s\n", payload);
}

void sendTelemetry(float temperature, float humidity, float soil)
{
    // String payload = "{\"temperature\":" + String(temperature, 1) +
    //                  ",\"humidity\":" + String(humidity, 1) +
    //                  ",\"soil\":" + String(soil, 1) + "}";

    // StaticJsonDocument<128> doc;
    JsonDocument doc;

    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["soil"] = soil;

    char payload[128];
    serializeJson(doc, payload);

    // Serial.print("📤2 Sending payload: ");
    // Serial.println(payload);
    client.publish("v1/devices/me/attributes", payload, true);
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
            client.publish("v1/devices/me/attributes/request/1", "{\"sharedKeys\":\"relays\"}");
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
