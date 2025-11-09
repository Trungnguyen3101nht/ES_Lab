#include "ledRGB.h"

// enum LedState
// {
//     LED_ERROR = 0,      // đỏ
//     LED_AP_MODE = 1,    // trắng nháy
//     LED_CONNECTING = 2, // vàng nháy
//     LED_OK = 3          // xanh
// };

volatile LedState currentLedState = LED_ERROR;

void LedTask(void *parameter)
{
    for (;;)
    {
        switch (currentLedState)
        {
        case LED_ERROR: // đỏ đứng yên
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            pixels.show();
            vTaskDelay(pdMS_TO_TICKS(200));
            break;

        case LED_AP_MODE: // trắng nhấp nháy
            pixels.setPixelColor(0, pixels.Color(255, 255, 255));
            pixels.show();
            vTaskDelay(pdMS_TO_TICKS(200));
            pixels.clear();
            pixels.show();
            vTaskDelay(pdMS_TO_TICKS(200));
            break;

        case LED_CONNECTING: // vàng nhịp chậm
            pixels.setPixelColor(0, pixels.Color(255, 255, 0));
            pixels.show();
            vTaskDelay(pdMS_TO_TICKS(150));
            pixels.clear();
            pixels.show();
            vTaskDelay(pdMS_TO_TICKS(850));
            break;

        case LED_OK: // xanh đứng yên
            pixels.setPixelColor(0, pixels.Color(0, 255, 0));
            pixels.show();
            vTaskDelay(pdMS_TO_TICKS(500));
            break;
        }
    }
}

void LedRGB_Init()
{
    xTaskCreatePinnedToCore(
        LedTask,
        "LedTask",
        2048,
        NULL,
        1,
        NULL,
        0);
}
