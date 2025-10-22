#ifndef LED_RGB_H
#define LED_RGB_H
#include "global.h"

void createLedRGB();
void LedRGB_Init();
void handleWSMesOfLED(void *arg, uint8_t *data, size_t len);
#endif // LED_RGB_H
