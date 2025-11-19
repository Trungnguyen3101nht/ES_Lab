#ifndef DEFINE_H
#define DEFINE_H

typedef struct
{
    int cmdType; // 1 = LED, 2 = RELAY, 3 = SENSOR temp/humi
    float Value01;
    float Value02;
} CommandMsg_t;
extern QueueHandle_t commandQueue;

#define REQUEST_QUEUE_LENGTH 10
#define UART_NUM UART_NUM_0
#define BUF_SIZE 128

#define LED_GPIO 45

// #define RESET_BUTTON_PIN 0    // GPIO0 (hoặc đổi pin bạn dùng)
// #define WIFI_TIMEOUT_MS 30000 // 30 giây không có Wi-Fi thì quay lại AP

extern const char *ssid;
extern const char *password;

extern AsyncWebServer server;
extern AsyncWebSocket ws;

extern Adafruit_NeoPixel pixels;

#define Relay_1 1
#define Relay_2 2
#define Relay_3 3
#define Relay_4 4

#endif // DEFINE_H