#ifndef SCHEDULE_H
#define SCHEDULE_H
#include "global.h"
#include "time.h"
#include "device/relay.h"

#define MAX_SCHEDULES 20

struct Schedule
{
    int relayIndex;
    bool state;
    int hour;
    int minute;
    uint8_t daysOfWeek;
};

void Schedule_Init();
void addSchedule(JsonDocument &doc);

#endif // SCHEDULE_H