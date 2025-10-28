#include "wifi.h"

Preferences preferences;

String ssid, password;
volatile bool shouldConnect = false;
volatile bool apActive = false;

// ================== HTML GIAO DIỆN CONFIG ==================
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
      margin: 0; padding: 0;
    }
    h2 { text-shadow: 0 0 10px #0ff; margin-top: 60px; }
    form {
      background: rgba(20,20,20,0.85);
      border: 2px solid #0ff;
      border-radius: 15px;
      padding: 30px;
      display: inline-block;
      margin-top: 40px;
    }
    input {
      margin: 10px; padding: 10px;
      background: #111; border: 1px solid #0ff;
      color: #0ff; border-radius: 5px; text-align: center;
    }
    input[type=submit] {
      background: linear-gradient(90deg,#00ffff,#ff00ff);
      color: black; font-weight: bold; border: none;
      border-radius: 8px; cursor: pointer;
    }
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

// ================== TASK NÚT RESET ==================
void buttonMonitorTask(void *parameter)
{
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  unsigned long pressStart = 0;
  bool pressed = false;

  for (;;)
  {
    if (digitalRead(RESET_BTN_PIN) == LOW)
    {
      if (!pressed)
      {
        pressStart = millis();
        pressed = true;
      }

      if (millis() - pressStart > RESET_HOLD_TIME)
      {
        Serial.println("⚠️ Giữ nút >5s → Xóa cấu hình WiFi!");
        preferences.begin("wifi", false);
        preferences.clear();
        preferences.end();

        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
        vTaskDelay(pdMS_TO_TICKS(500));

        shouldConnect = false;
        apActive = false;
        pixels.clear();
        pixels.show();

        xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 4, NULL, 1);
        vTaskDelete(NULL);
      }
    }
    else
      pressed = false;

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ================== TASK KẾT NỐI WIFI ==================
void connectWiFiTask(void *parameter)
{
  Serial.println("🔄 Kết nối tới WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  for (int retry = 0; WiFi.status() != WL_CONNECTED && retry < 20; retry++)
  {
    pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // vàng nhấp nháy
    pixels.show();
    vTaskDelay(pdMS_TO_TICKS(150));
    pixels.clear();
    pixels.show();
    vTaskDelay(pdMS_TO_TICKS(850));
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // xanh lá
    pixels.show();

    Serial.printf("\n✅ Đã kết nối WiFi!\nIP: %s\n", WiFi.localIP().toString().c_str());

    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", password);
    preferences.end();
    Serial.println("💾 Đã lưu thông tin WiFi!");

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
    shouldConnect = apActive = false;
    vTaskDelay(pdMS_TO_TICKS(1500));
    xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 4, NULL, 1);
  }

  vTaskDelete(NULL);
}

// ================== TASK ACCESS POINT ==================
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

  // LED trắng nhấp nháy khi ở AP mode
  xTaskCreatePinnedToCore([](void *)
                          {
    while (apActive) {
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.show(); vTaskDelay(pdMS_TO_TICKS(300));
      pixels.clear(); pixels.show(); vTaskDelay(pdMS_TO_TICKS(300));
    }
    vTaskDelete(NULL); }, "apLedBlinkTask", 3072, NULL, 3, NULL, 1);

  Serial.printf("📶 Access Point đã bật! IP: %s\n", WiFi.softAPIP().toString().c_str());

  serverAP.on("/", HTTP_GET, [](AsyncWebServerRequest *req)
              { req->send(200, "text/html", htmlPage); });
  serverAP.on("/save", HTTP_POST, [](AsyncWebServerRequest *req)
              {
    if (req->hasParam("ssid", true) && req->hasParam("pass", true)) {
      ssid = req->getParam("ssid", true)->value();
      password = req->getParam("pass", true)->value();
      req->send(200, "text/html", "✅ Đã lưu WiFi! ESP32 sẽ kết nối...");
      Serial.printf("📥 SSID: %s | PASS: %s\n", ssid.c_str(), password.c_str());
      shouldConnect = true;
    } else req->send(400, "text/plain", "Thiếu SSID hoặc mật khẩu!"); });

  serverAP.begin();
  Serial.println("🌐 WebServer (AP Mode) đã sẵn sàng!");

  for (;;)
  {
    if (shouldConnect)
    {
      shouldConnect = apActive = false;
      pixels.clear();
      pixels.show();

      Serial.println("🔻 Tắt Access Point...");
      serverAP.end();
      WiFi.softAPdisconnect(true);
      vTaskDelay(pdMS_TO_TICKS(300));

      Serial.println("🚀 Bắt đầu task kết nối WiFi...");
      xTaskCreatePinnedToCore(connectWiFiTask, "connectWiFiTask", 8192, NULL, 4, NULL, 1);
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ================== KHỞI TẠO WIFI ==================
void Wifi_init()
{
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  xTaskCreatePinnedToCore(buttonMonitorTask, "buttonMonitorTask", 4096, NULL, 5, NULL, 1);

  preferences.begin("wifi", true);
  String savedSSID = preferences.getString("ssid", "");
  String savedPASS = preferences.getString("pass", "");
  preferences.end();

  if (savedSSID.isEmpty())
  {
    Serial.println("⚙️ Không có WiFi đã lưu, khởi động AP...");
    xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 4, NULL, 1);
  }
  else
  {
    ssid = savedSSID;
    password = savedPASS;
    Serial.printf("📡 Đã tìm thấy WiFi đã lưu: %s\n", ssid.c_str());
    xTaskCreatePinnedToCore(connectWiFiTask, "connectWiFiTask", 8192, NULL, 4, NULL, 1);
  }
}
