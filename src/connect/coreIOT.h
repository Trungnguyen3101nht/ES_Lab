#ifndef CORE_IOT_H
#define CORE_IOT_H
#include "global.h"
#include "../device/soil.h"
#include "../device/tempandhumi.h"

extern PubSubClient client;
void sendRelayStateToCore();
void coreIOT_init();
#endif // CORE_IOT_H