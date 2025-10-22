#include "webserver.h"

TaskHandle_t TaskWebServerHandle;

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
        {
            data[len] = 0;
            String message = (char *)data;

            // Serial.print("Received: ");
            Serial.println(message);
            if (message.startsWith("LED"))
            {
                handleWSMesOfLED(arg, data, len);
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
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Trang HTML chính
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });

    // File CSS
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/styles.css", "text/css"); });

    // File JavaScript
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/script.js", "application/javascript"); });

    server.begin();
    Serial.println("WebSocket server started with RTOS!");
}

void TaskWebServer(void *pvParameters)
{
    initWebServer();
    for (;;)
    {
        ws.cleanupClients();
        vTaskDelay(pdMS_TO_TICKS(50)); // Giải phóng CPU
    }
}

void webServer_Init()
{
    xTaskCreatePinnedToCore(
        TaskWebServer,
        "WebServerTask",
        8192,
        NULL,
        2,
        &TaskWebServerHandle,
        1);
}