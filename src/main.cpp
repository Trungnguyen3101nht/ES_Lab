#include "global.h"

const char *ssid = "ACLAB";
const char *password = "ACLAB2023";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Adafruit_NeoPixel pixels(1, LED_GPIO, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);

  // Mount SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS mount failed!");
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  Connect_Init();
  Device_Init();
}

void loop()
{
  // Serial.println(WiFi.localIP());
  // delay(10000);
}
