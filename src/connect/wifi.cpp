#include "wifi.h"

String ssid = "";
String password = "";
volatile bool shouldConnect = false;
volatile bool apActive = false;

const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 WiFi Config</title>
  <style>
    body {
      background: radial-gradient(circle at top, #111, #000);
      color: #0ff;
      font-family: 'Orbitron', sans-serif;
      text-align: center;
    }
    h2 { text-shadow: 0 0 10px #0ff; }
    form { background: rgba(20,20,20,0.85); border: 2px solid #0ff; border-radius: 15px; padding: 30px; display: inline-block; margin-top: 40px; }
    input { margin: 10px; padding: 10px; background: #111; border: 1px solid #0ff; color: #0ff; border-radius: 5px; }
    input[type=submit]{ background: linear-gradient(90deg,#00ffff,#ff00ff); color: black; font-weight: bold; border:none; border-radius: 8px; cursor:pointer; }
  </style>
</head>
<body>
  <h2>⚡ ESP32 WiFi Config ⚡</h2>
  <form action="/save" method="post">
    <input type="text" name="ssid" placeholder="SSID"><br>
    <input type="password" name="pass" placeholder="Password"><br>
    <input type="submit" value="Lưu">
  </form>
</body>
</html>
)rawliteral";

void connectWiFiTask(void *parameter)
{
  Serial.println("🔄 Kết nối tới WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20)
  {
    pixels.setPixelColor(0, pixels.Color(255, 255, 0));
    pixels.show();
    vTaskDelay(pdMS_TO_TICKS(150));
    pixels.clear();
    pixels.show();
    vTaskDelay(pdMS_TO_TICKS(850));
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();

    Serial.println("\n✅ Đã kết nối WiFi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    serverAP.end();
    vTaskDelay(pdMS_TO_TICKS(100));

    webServer_Init();
  }
  else
  {
    Serial.println("\n❌ Kết nối thất bại! Quay lại AP mode...");

    for (int i = 0; i < 6; i++)
    {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
      vTaskDelay(pdMS_TO_TICKS(200));
      pixels.clear();
      pixels.show();
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    shouldConnect = false;
    apActive = false;
    vTaskDelay(pdMS_TO_TICKS(1500));

    xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 5, NULL, 1);
  }

  vTaskDelete(NULL);
}

void apTask(void *parameter)
{
  if (apActive)
  {
    vTaskDelete(NULL);
    return;
  }

  apActive = true;

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_Config", "12345678");

  Serial.println("📶 Access Point đã bật!");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  serverAP.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", htmlPage); });

  serverAP.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
              {
                 if (request->hasParam("ssid", true) && request->hasParam("pass", true))
                 {
                   ssid = request->getParam("ssid", true)->value();
                   password = request->getParam("pass", true)->value();

                   request->send(200, "text/html", "✅ Đã lưu WiFi! ESP32 sẽ kết nối...");
                   Serial.println("📥 SSID: " + ssid);
                   Serial.println("PASS: " + password);

                   shouldConnect = true;
                 }
                 else
                 {
                   request->send(400, "text/plain", "Thiếu SSID hoặc mật khẩu!");
                 } });

  serverAP.begin();
  Serial.println("🌐 WebServer (AP Mode) đã sẵn sàng!");

  while (true)
  {
    if (shouldConnect)
    {
      shouldConnect = false;
      apActive = false;

      Serial.println("🔻 Tắt Access Point...");
      serverAP.end();
      WiFi.softAPdisconnect(true);
      vTaskDelay(pdMS_TO_TICKS(300));

      Serial.println("🚀 Bắt đầu task kết nối WiFi...");
      xTaskCreatePinnedToCore(connectWiFiTask, "connectWiFiTask", 8192, NULL, 5, NULL, 1);

      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void Wifi_init()
{
  xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 5, NULL, 1);
}
