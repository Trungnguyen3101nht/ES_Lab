#include "schedule.h"

Schedule schedules[MAX_SCHEDULES];
int scheduleCount = 0;
int lastExecutedMinute[MAX_SCHEDULES] = {-1};

bool matchDayOfWeek(uint8_t mask, int wday)
{
    return mask & (1 << wday);
}

void TaskSchedule(void *pvParameters)
{
    for (;;)
    {
        xSemaphoreTake(timeMutex, portMAX_DELAY);
        int nowHour = currentTime.tm_hour;
        int nowMin = currentTime.tm_min;
        int nowWday = currentTime.tm_wday;
        xSemaphoreGive(timeMutex);

        for (int i = 0; i < scheduleCount; i++)
        {
            Schedule &s = schedules[i];

            if (!matchDayOfWeek(s.daysOfWeek, nowWday))
                continue;

            if (s.hour == nowHour && s.minute == nowMin)
            {
                if (lastExecutedMinute[i] != nowMin)
                {
                    lastExecutedMinute[i] = nowMin;

                    digitalWrite(relayPins[s.relayIndex], s.state ? HIGH : LOW);
                    relayState[s.relayIndex] = s.state;
                    writeRelayState();

                    Serial.printf("⏰ Schedule HIT → Relay %d %s\n",
                                  s.relayIndex + 1,
                                  s.state ? "ON" : "OFF");
                }
            }
                }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void addSchedule(JsonDocument &doc)
{
    if (!doc["schedule"].is<JsonObject>())
        return; // phải là object mới add

    JsonObject obj = doc["schedule"];

    Schedule s;
    s.relayIndex = obj["relay"].as<int>();
    s.state = obj["state"].as<bool>();
    s.hour = obj["hour"].as<int>();
    s.minute = obj["minute"].as<int>();
    s.daysOfWeek = 0;
    JsonArray days = obj["days"];
    for (int d : days)
        s.daysOfWeek |= (1 << d);

    if (scheduleCount < MAX_SCHEDULES)
    {
        schedules[scheduleCount++] = s;
        Serial.println("✅ Schedule added");

        // gửi danh sách schedule về client
        DynamicJsonDocument outDoc(1024);

        JsonArray arr = outDoc.createNestedArray("schedules");
        for (int i = 0; i < scheduleCount; i++)
        {
            JsonObject o = arr.createNestedObject();
            o["relay"] = schedules[i].relayIndex + 1;
            o["state"] = schedules[i].state;
            o["hour"] = schedules[i].hour;
            o["minute"] = schedules[i].minute;

            JsonArray d = o.createNestedArray("days");
            for (int j = 0; j < 7; j++)
                if (schedules[i].daysOfWeek & (1 << j))
                    d.add(j);
        }

        String outStr;
        serializeJson(outDoc, outStr);
        ws.textAll(outStr);
    }
}

void Schedule_Init()
{
    xTaskCreatePinnedToCore(TaskSchedule, "TaskSchedule", 4096, NULL, 3, NULL, 1);
}
