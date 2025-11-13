#include "wifi.h"

Preferences preferences;

String ssid, password;
volatile bool shouldConnect = false;
volatile bool apActive = false;

// ================== TASK NÚT RESET ==================
void buttonMonitorTask(void *parameter)
{
  currentLedState = LED_ERROR;

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
  // WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  for (int retry = 0; WiFi.status() != WL_CONNECTED && retry < 20; retry++)
  {

    currentLedState = LED_CONNECTING;
    Serial.print(".");
    vTaskDelay(pdMS_TO_TICKS(500)); // delay 0.5s mỗi lần thử
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    //?/
    currentLedState = LED_OK;
    vTaskDelay(100);
    Serial.printf("\n✅ Đã kết nối WiFi!\nIP: %s\n", WiFi.localIP().toString().c_str());

    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", password);
    preferences.end();
    Serial.println("💾 Đã lưu thông tin WiFi!");
    serverAP.reset();
    serverAP.end();
    vTaskDelay(pdMS_TO_TICKS(100));
    webServer_Init();
  }
  // else
  // {
  //   Serial.println("\n❌ Kết nối thất bại! Quay lại AP mode...");

  //   //?/
  //   currentLedState = LED_ERROR;

  //   shouldConnect = false;
  //   apActive = false;

  //   Serial.println("🔄 Chuyển sang Access Point mode...");
  //   vTaskDelay(pdMS_TO_TICKS(2000));
  //   xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 4, NULL, 1);
  // }

  vTaskDelete(NULL);
}
void wifiMonitorTask(void *parameter)
{
  for (;;)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("⚠️ WiFi bị mất kết nối!");
      currentLedState = LED_ERROR;
      // 🔴 báo đỏ
      int retry = 0;
      WiFi.reconnect();
      while (WiFi.status() != WL_CONNECTED && retry < 30)
      {
        currentLedState = LED_CONNECTING;
        // 🟡 nhấp nháy trong lúc thử kết nối lại
        vTaskDelay(pdMS_TO_TICKS(50));
        retry++;
      }
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("✅ WiFi đã kết nối lại!");
        currentLedState = LED_OK;
        // 🟢 trở lại trạng thái bình thường
      }
      else
      {
        Serial.println("❌ Không reconnect được → CHỈ quay lại AP mode nếu MQTT không chạy");

        // ✅ Chỉ quay về AP nếu MQTT task chưa chạy
        if (!client.connected())
        {
          xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 3, NULL, 0);
          vTaskDelete(NULL);
        }
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
  }
  apActive = true;

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_ID, AP_PASS);
  vTaskDelay(pdMS_TO_TICKS(500));
  // LED trắng nhấp nháy khi ở AP mode
  if (!LittleFS.begin(true))
  {
    Serial.println("⚠️ Mount LittleFS thất bại!");
    return;
  }
  if (!LittleFS.exists("/AP_index.html"))
  {
    Serial.println("⚠️ AP_index.html không tồn tại!");
  }

  currentLedState = LED_AP_MODE;

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
      
      Serial.printf("📥 SSID: %s | PASS: %s\n", ssid.c_str(), password.c_str());
      shouldConnect = true;
 

    req->send(200, "text/html", "Check the Serial Monitor for connection status.");
    } else req->send(400, "text/plain", "Thiếu SSID hoặc mật khẩu!"); });

  serverAP.begin();
  Serial.println("🌐 WebServer (AP Mode) đã sẵn sàng!");

  for (;;)
  {
    if (shouldConnect)
    {
      shouldConnect = false;
      apActive = false;

      Serial.println("🔻 Tắt Access Point...");

      serverAP.reset();
      serverAP.end();
      WiFi.softAPdisconnect(true);
      vTaskDelay(pdMS_TO_TICKS(300));

      Serial.println("🚀 Bắt đầu task kết nối WiFi...");
      xTaskCreatePinnedToCore(connectWiFiTask, "connectWiFiTask", 8192, NULL, 4, NULL, 0);
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ================== KHỞI TẠO WIFI ==================
void Wifi_init()
{
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  xTaskCreatePinnedToCore(buttonMonitorTask, "buttonMonitorTask", 4096, NULL, 2, NULL, 0);

  preferences.begin("wifi", true);
  String savedSSID = preferences.getString("ssid", "");
  String savedPASS = preferences.getString("pass", "");
  preferences.end();

  if (savedSSID.isEmpty())
  {
    Serial.println("⚙️ Không có WiFi đã lưu, khởi động AP...");
    xTaskCreatePinnedToCore(apTask, "apTask", 8192, NULL, 2, NULL, 0);
  }
  else
  {
    ssid = savedSSID;
    password = savedPASS;
    Serial.printf("📡 Đã tìm thấy WiFi đã lưu: %s\n", ssid.c_str());
    xTaskCreatePinnedToCore(connectWiFiTask, "connectWiFiTask", 8192, NULL, 4, NULL, 0);
  }
  xTaskCreatePinnedToCore(wifiMonitorTask, "wifiMonitorTask", 4096, NULL, 3, NULL, 0);
}
