#include "global.h"

// WiFi credentials
const char *ssid = "ACLAB";
const char *password = "ACLAB2023";

// FreeRTOS handles
TaskHandle_t TaskWebServerHandle;
TaskHandle_t TaskGPIOHandle;

// Shared variable (RTOS-safe)
volatile bool ledCommand = false;
volatile bool newCommand = false;

Adafruit_NeoPixel pixels(1, LED_GPIO, NEO_GRB + NEO_KHZ800);

// Forward declarations
void TaskWebServer(void *pvParameters);
void TaskGPIO(void *pvParameters);

void setup()
{
  Serial.begin(115200);
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);

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

  pixels.begin();            // KHỞI TẠO bắt buộc
  pixels.setBrightness(255); // độ sáng 0–255
  pixels.clear();            // xóa tất cả
  pixels.show();             // cập nhật để tắt ban đầu
  Serial.println("Setup done");

  // Create RTOS tasks
  xTaskCreatePinnedToCore(
      TaskWebServer,
      "WebServerTask",
      8192,
      NULL,
      1,
      &TaskWebServerHandle,
      1);

  xTaskCreatePinnedToCore(
      TaskGPIO,
      "GPIOControlTask",
      2048,
      NULL,
      1,
      &TaskGPIOHandle,
      0);
}

void loop()
{
  // FreeRTOS sẽ lo việc chạy song song các task
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS mount failed!");
    return;
  }
  Serial.println("SPIFFS mounted successfully!");
  delay(10000);
}

// Task: chạy WebSocket server
