#include "connect_init.h"
#include "webserver.h"
#include "wifi.h"
void Connect_Init()
{
    webServer_Init();
    Wifi_init();
}