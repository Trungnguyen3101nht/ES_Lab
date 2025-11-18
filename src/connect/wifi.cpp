#include "wifi.h"

Preferences preferences;

String ssid, password;
volatile bool shouldConnect = false;
volatile bool apActive = false;

void Led_control(int Ledstate)
{
  if (Ledstate == 0)
  {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
  }
  if (Ledstate == 1) // thiết bị đang ở trạng thái accesspoint
  {
    for (int i = 0; i < 6; i++)
    {
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.show();
      vTaskDelay(pdMS_TO_TICKS(200));
      pixels.clear();
      pixels.show();
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
  else if (Ledstate == 2) // thiết bị đang kết nối wifi
  {
    pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // vàng nhấp nháy
    pixels.show();
    vTaskDelay(pdMS_TO_TICKS(150));
    pixels.clear();
    pixels.show();
    vTaskDelay(pdMS_TO_TICKS(850));
  }
  else if (Ledstate == 3)
  {
    // thiết bị đang kết nối wifi
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // xanh lá
    pixels.show();
  }
}
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

  // WiFi.mode(WIFI_STA);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  for (int retry = 0; WiFi.status() != WL_CONNECTED && retry < 20; retry++)
  {
    //?/
    Led_control(2); // LED vàng nhấp nháy khi đang kết nối
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    //?/
    Led_control(3); // LED xanh lá khi đã kết nối thành công
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

    //?/
    Led_control(0);
    shouldConnect = false;
    apActive = false;
    pixels.clear();
    pixels.show();
    Serial.println("🔄 Chuyển sang Access Point mode...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 4, NULL, 1);
  }

  vTaskDelete(NULL);
}
void wifiMonitorTask(void *parameter)
{
  for (;;)
  {
    if (WiFi.getMode() == WIFI_STA)
    { // chỉ kiểm tra nếu đang ở chế độ STA
      if (WiFi.status() != WL_CONNECTED)
      {
        Serial.println("⚠️ WiFi bị mất kết nối!");
        Led_control(0); // 🔴 báo đỏ
        int retry = 0;
        WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED && retry < 20)
        {
          Led_control(2); // 🟡 nhấp nháy trong lúc thử kết nối lại
          vTaskDelay(pdMS_TO_TICKS(500));
          retry++;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          Serial.println("✅ WiFi đã kết nối lại!");
          Led_control(3); // 🟢 trở lại trạng thái bình thường
        }
        // else
        // {
        //   Serial.println("❌ Không kết nối lại được → quay về AP mode!");
        //   WiFi.disconnect(true, true);
        //   WiFi.mode(WIFI_OFF);
        //   vTaskDelay(pdMS_TO_TICKS(500));
        //   xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 4, NULL, 1);
        //   vTaskDelete(NULL); // dừng monitor cũ
        // }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5000)); // kiểm tra mỗi 5 giây
  }
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
  WiFi.softAP(AP_ID, AP_PASS);
  vTaskDelay(pdMS_TO_TICKS(500));
  // LED trắng nhấp nháy khi ở AP mode
  xTaskCreatePinnedToCore([](void *)
                          {
    while (apActive) {
      // pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      // pixels.show(); vTaskDelay(pdMS_TO_TICKS(300));
      // pixels.clear(); pixels.show(); vTaskDelay(pdMS_TO_TICKS(300));
      Led_control(1);
    }
    vTaskDelete(NULL); }, "apLedBlinkTask", 3072, NULL, 3, NULL, 1);

  Serial.printf("📶 Access Point đã bật! IP: %s\n", WiFi.softAPIP().toString().c_str());

  serverAP.on("/", HTTP_GET, [](AsyncWebServerRequest *req)
              { req->send(LittleFS, "/AP_index.html", "text/html"); });
  serverAP.on("/scan", HTTP_GET, [](AsyncWebServerRequest *req)
              {
  int n = WiFi.scanNetworks();
  if (n == 0) {
    req->send(200, "application/json", "[]");
    return;
  }

  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i) json += ",";
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
    json += "}";
  }
  json += "]";
  req->send(200, "application/json", json);
  WiFi.scanDelete(); });

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
      shouldConnect = false;
      apActive = false;
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
  xTaskCreatePinnedToCore(wifiMonitorTask, "wifiMonitorTask", 4096, NULL, 3, NULL, 1);
}
