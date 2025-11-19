#ifndef LED_RGB_H
#define LED_RGB_H
#include "global.h"

// ledRGB.h
extern bool ledState;

void createLedRGB();
void LedRGB_Init();
void writeLedstate();
void handleWSMesOfLED(void *arg, uint8_t *data, size_t len);
#endif // LED_RGB_H
