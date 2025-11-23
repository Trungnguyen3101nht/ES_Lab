#ifndef RELAY_H
#define RELAY_H
#include "global.h"
#include "task/schedule.h"
extern bool relayState[NUM_RELAYS];
extern const int relayPins[NUM_RELAYS];

void handleRelayUpdate(const char *payload);
void writeRelayState();
void Relay_Init();
void handleWSMesOfRelay(void *arg, uint8_t *data, size_t len);

#endif // RELAY_H
