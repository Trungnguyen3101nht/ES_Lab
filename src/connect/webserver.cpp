#include "webserver.h"

TaskHandle_t TaskWebServerHandle;
void waitForWiFi()
{
    Serial.print("⏳ Đang kết nối WiFi");
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30)
    {
        delay(500);
        Serial.print(".");
        retries++;
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("✅ WiFi đã sẵn sàng, IP: ");
        Serial.println(WiFi.localIP());
        currentLedState = LED_OK;
    }
    else
    {
        Serial.println("❌ Không thể kết nối WiFi!");
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->opcode == WS_TEXT)
        {
            String message;
            for (size_t i = 0; i < len; i++)
                message += (char)data[i];

            Serial.print("📥 Received WS: ");
            Serial.println(message);

            if (message.startsWith("{"))
            {
                DynamicJsonDocument doc(512);
                DeserializationError error = deserializeJson(doc, message);
                if (error)
                {
                    Serial.print("❌ JSON parse failed: ");
                    Serial.println(error.c_str());
                }
                else
                {
                    addSchedule(doc);
                }
            }
            else if (message.startsWith("RELAY"))
            {
                handleWSMesOfRelay(arg, data, len);
            }
        }
    }
}
void initWebServer()
{
    // if (!LittleFS.begin(true))
    // {
    //     Serial.println("LittleFS mount failed!");
    //     return;
    // }
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("⚠️ WiFi chưa sẵn sàng, chưa khởi động WebServer!");
        return;
    }

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });

    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });

    server.begin();
    Serial.println("🌍 WebSocket server started!");
}

void TaskWebServer(void *pvParameters)
{
    waitForWiFi();
    initWebServer();
    for (;;)
    {
        ws.cleanupClients();
        vTaskDelay(pdMS_TO_TICKS(50)); // Giải phóng CPU
    }
}

void webServer_Init()
{
    WiFi.mode(WIFI_STA); // BẮT BUỘC

    xTaskCreatePinnedToCore(
        TaskWebServer,
        "WebServerTask",
        8192,
        NULL,
        3,
        &TaskWebServerHandle,
        0);
}