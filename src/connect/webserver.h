#ifndef WEB_SERVER_H
#define WEB_SERVER_H
#include "global.h"

void initWebServer();
void notifyClients();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len);
void initWebServer();

#endif // WEB_SERVER_H