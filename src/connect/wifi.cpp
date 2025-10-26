// #include "wifi.h"

// Preferences prefs;
// WebServer server(80);

// const char *apSSID = "ESP32_Config_AP";
// const char *apPassword = "12345678";

// unsigned long wifiLostTime = 0;
// bool wifiConnected = false;
// bool apMode = false;

// void startAccessPoint()
// {
//     WiFi.mode(WIFI_AP);
//     WiFi.softAP(apSSID, apPassword);
//     IPAddress myIP = WiFi.softAPIP();
//     Serial.printf("[AP Mode] SSID: %s | IP: %s\n", apSSID, myIP.toString().c_str());
//     apMode = true;

//     server.on("/", HTTP_GET, []()
//               {
//     String html = R"rawliteral(
//       <html><body>
//       <h2>ESP32 Wi-Fi Config</h2>
//       <form action="/save" method="post">
//       SSID: <input name="ssid"><br>
//       Password: <input name="pass" type="password"><br>
//       <input type="submit" value="Lưu cấu hình">
//       </form></body></html>
//     )rawliteral";
//     server.send(200, "text/html; charset=utf-8", html); });

//     server.on("/save", HTTP_POST, []()
//               {
//     String ssid = server.arg("ssid");
//     String pass = server.arg("pass");

//     prefs.begin("wifi", false);
//     prefs.putString("ssid", ssid);
//     prefs.putString("pass", pass);
//     prefs.end();

//     server.send(200, "text/html; charset=utf-8",
//                 "<h3>Đã lưu Wi-Fi! ESP sẽ khởi động lại...</h3>");
//     delay(1500);
//     ESP.restart(); });

//     server.begin();
//     Serial.println("[AP Mode] WebServer chạy tại http://192.168.4.1");
// }

// // =============================
// void WiFiEvent(WiFiEvent_t event)
// {
//     switch (event)
//     {
//     case SYSTEM_EVENT_STA_GOT_IP:
//         Serial.print("[WiFi] Kết nối thành công! IP: ");
//         Serial.println(WiFi.localIP());
//         wifiConnected = true;
//         wifiLostTime = 0;
//         break;

//     case SYSTEM_EVENT_STA_DISCONNECTED:
//         Serial.println("[WiFi] Mất kết nối!");
//         wifiConnected = false;
//         wifiLostTime = millis();
//         break;

//     default:
//         break;
//     }
// }

// void connectToSavedWiFi()
// {
//     prefs.begin("wifi", true);
//     String ssid = prefs.getString("ssid", "");
//     String pass = prefs.getString("pass", "");
//     prefs.end();

//     if (ssid == "")
//     {
//         Serial.println("[WiFi] Không có cấu hình, bật AP...");
//         startAccessPoint();
//         return;
//     }

//     WiFi.mode(WIFI_STA);
//     WiFi.onEvent(WiFiEvent);
//     WiFi.begin(ssid.c_str(), pass.c_str());

//     Serial.printf("[WiFi] Đang kết nối tới SSID: %s\n", ssid.c_str());
// }

// void clearWiFiConfig()
// {
//     prefs.begin("wifi", false);
//     prefs.clear();
//     prefs.end();
//     Serial.println("[WiFi] Đã xóa cấu hình Wi-Fi!");
// }

// void checkResetButton()
// {
//     static unsigned long pressedTime = 0;
//     static bool pressed = false;

//     if (digitalRead(RESET_BUTTON_PIN) == LOW)
//     { // nhấn
//         if (!pressed)
//         {
//             pressed = true;
//             pressedTime = millis();
//         }
//         else if (millis() - pressedTime > 10000)
//         { // giữ > 10 giây
//             Serial.println("[BUTTON] Giữ nút 10s: xóa Wi-Fi và bật lại AP!");
//             clearWiFiConfig();
//             delay(500);
//             ESP.restart();
//         }
//     }
//     else
//     {
//         pressed = false;
//     }
// }

// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);
//     pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

//     Serial.println("=== ESP32 Wi-Fi Auto Reconfig ===");
//     connectToSavedWiFi();
// }

// void loop()
// {
//     if (apMode)
//     {
//         server.handleClient(); // xử lý web khi đang ở AP
//     }
//     else
//     {
//         checkResetButton();

//         // Nếu mất Wi-Fi quá 30 giây => restart về AP
//         if (!wifiConnected && wifiLostTime > 0 && (millis() - wifiLostTime > WIFI_TIMEOUT_MS))
//         {
//             Serial.println("[WiFi] Mất Wi-Fi quá lâu, quay lại Access Point...");
//             clearWiFiConfig();
//             delay(500);
//             ESP.restart();
//         }
//     }

//     delay(10);
// }
