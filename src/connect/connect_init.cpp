#include "connect_init.h"

void Connect_Init()
{

    Wifi_init();
    webServer_Init();
    coreIOT_init();
}