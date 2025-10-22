#ifndef RELAY_H
#define RELAY_H
#include "global.h"

void Relay_Init();
void handleWSMesOfRelay(void *arg, uint8_t *data, size_t len); // ✅ thêm dòng này

#endif // RELAY_H
