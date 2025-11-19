#include "global.h"

const char *ssid = "Nhaxe 42/36/21A";
const char *password = "88888888";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Adafruit_NeoPixel pixels(1, LED_GPIO, NEO_GRB + NEO_KHZ800);
QueueHandle_t commandQueue = NULL;

void setup()
{
  Wire.begin(MY_SCL, MY_SDA);
  Wire.setClock(100000);

  Serial.begin(115200);
  commandQueue = xQueueCreate(20, sizeof(CommandMsg_t));
  if (!LittleFS.begin(true))
  {
    Serial.println("LittleFS mount failed!");
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
  Serial.println(WiFi.localIP());
  delay(10000);
}
