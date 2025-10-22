#ifndef DEFINE_H
#define DEFINE_H

#define REQUEST_QUEUE_LENGTH 10
#define UART_NUM UART_NUM_0
#define BUF_SIZE 128

#define LED_GPIO 45

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