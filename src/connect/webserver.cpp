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
            String message = String((char *)data, len);

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
void TaskProcessCommands(void *pv)
{
    CommandMsg_t cmd;
    for (;;)
    {
        if (xQueueReceive(commandQueue, &cmd, portMAX_DELAY) == pdPASS)
        {
            switch (cmd.cmdType)
            {
            case 1: // LED
                ledState = cmd.Value01 != 0;
                if (ledState)
                    pixels.setPixelColor(0, pixels.Color(255, 0, 255));
                else
                    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
                pixels.show();
                writeLedstate();
                break;

            case 2: // RELAY
                digitalWrite(relayPins[(int)cmd.Value01], cmd.Value02 ? HIGH : LOW);
                relayState[(int)cmd.Value01] = cmd.Value02;
                writeRelayState();
                break;

            case 3: // SENSOR
            {
                String json = "{\"temperature\":" + String(cmd.Value01, 2) +
                              ",\"humidity\":" + String(cmd.Value02, 2) + "}";
                if (ws.count() > 0)
                    ws.textAll(json);
                break;
            }

            default:
                break;
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
              { request->send(LittleFS, "/index.html", "text/html"); });

    // File CSS
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });

    // File JavaScript
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });

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
    xTaskCreatePinnedToCore(TaskProcessCommands, "ProcessCmds", 4096, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(
        TaskWebServer,
        "WebServerTask",
        8192,
        NULL,
        2,
        &TaskWebServerHandle,
        1);
}