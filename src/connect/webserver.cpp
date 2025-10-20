#include "webserver.h"

extern Adafruit_NeoPixel pixels;
extern volatile bool ledCommand;
extern volatile bool newCommand;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool ledState = false;

void notifyClients()
{
    DynamicJsonDocument doc(128);
    doc["led"] = ledState;
    String json;
    serializeJson(doc, json);
    ws.textAll(json);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        String msg = (char *)data;
        if (msg == "toggle")
        {
            ledState = !ledState;
            ledCommand = ledState;
            newCommand = true;
        }
    }
}

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

            Serial.print("Received: ");
            Serial.println(message);

            if (message == "ON")
            {
                pixels.setPixelColor(0, pixels.Color(255, 0, 0));
                pixels.show();
                Serial.println("LED ON");
            }
            else if (message == "OFF")
            {
                pixels.setPixelColor(0, pixels.Color(0, 0, 0));
                pixels.show();
                Serial.println("LED OFF");
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
        vTaskDelay(pdMS_TO_TICKS(100)); // Giải phóng CPU
    }
}

// Task: điều khiển GPIO dựa trên lệnh
void TaskGPIO(void *pvParameters)
{
    for (;;)
    {
        if (newCommand)
        {
            digitalWrite(LED_GPIO, ledCommand ? HIGH : LOW);
            notifyClients();
            newCommand = false;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // delay nhỏ để tránh busy loop
    }
}
