#ifndef TIME_H
#define TIME_H

#include "global.h"
extern struct tm currentTime;
extern SemaphoreHandle_t timeMutex;
void Time_Init();

#endif // TIME_H