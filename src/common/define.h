#ifndef DEFINE_H
#define DEFINE_H

extern AsyncWebServer server;
extern AsyncWebServer serverAP;

extern AsyncWebSocket ws;

extern Adafruit_NeoPixel pixels;

#define AP_ID "YoloUno_Config_"
#define AP_PASS "88888888"

#define REQUEST_QUEUE_LENGTH 10
#define UART_NUM UART_NUM_0
#define BUF_SIZE 128

#define RESET_BTN_PIN 0      // Nút BOOT
#define RESET_HOLD_TIME 5000 // Giữ 5s để reset WiFi

#define LED_GPIO 45
#define Relay_1 1
#define Relay_2 2
#define Relay_3 3
#define Relay_4 4

#endif // DEFINE_H