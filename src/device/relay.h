#ifndef RELAY_H
#define RELAY_H
#include "global.h"

const int relayPins[4] = {Relay_1, Relay_2, Relay_3, Relay_4};

extern bool relayState[4];

void Relay_Init();
void writeRelayState();
void handleWSMesOfRelay(void *arg, uint8_t *data, size_t len);

#endif // RELAY_H
