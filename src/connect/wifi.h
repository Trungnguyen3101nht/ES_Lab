#ifndef WIFI_H
#define WIFI_H
#pragma once
#include <global.h>
#include "webserver.h"
extern AsyncWebServer server;

void Wifi_init();
void apTask(void *parameter);
void connectWiFiTask(void *parameter);
#endif // WIFI_H