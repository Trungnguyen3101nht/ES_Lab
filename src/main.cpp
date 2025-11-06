#include "global.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebServer serverAP(8080);
Adafruit_NeoPixel pixels(1, LED_GPIO, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);

  Connect_Init();
  Device_Init();
}

void loop()
{
  Serial.println(WiFi.localIP());
  delay(10000);
}
