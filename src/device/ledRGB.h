#ifndef LED_RGB_H
#define LED_RGB_H
#include "global.h"

enum LedState
{
    LED_ERROR = 0,
    LED_AP_MODE = 1,
    LED_CONNECTING = 2,
    LED_OK = 3
};

// ✅ PUBLIC biến trạng thái LED
extern volatile LedState currentLedState;

void LedRGB_Init();
#endif // LED_RGB_H
