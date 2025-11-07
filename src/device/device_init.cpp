#include "device_init.h"

void Device_Init()
{
    LedRGB_Init();
    Relay_Init();
    TempandHumi_init();
    SoilSensor_Init();
}