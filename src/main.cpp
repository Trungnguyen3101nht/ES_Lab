#include <Arduino.h>

#define TIMER1_PERIOD_MS 2000 // 2 seconds
#define TIMER2_PERIOD_MS 3000 // 3 seconds

// Number of prints before stopping
#define TIMER1_MAX_COUNT 10
#define TIMER2_MAX_COUNT 5

// Timer handles
TimerHandle_t timer1_handle;
TimerHandle_t timer2_handle;

// Shared callback function
void timer_callback(TimerHandle_t xTimer)
{
  static int count1 = 0;
  static int count2 = 0;

  if (xTimer == timer1_handle)
  {
    count1++;
    printf("ahihi\n");

    if (count1 >= TIMER1_MAX_COUNT)
    {
      xTimerStop(timer1_handle, 0);
      printf("Timer 1 stopped.\n");
    }
  }
  else if (xTimer == timer2_handle)
  {
    count2++;
    printf("ihaha\n");

    if (count2 >= TIMER2_MAX_COUNT)
    {
      xTimerStop(timer2_handle, 0);
      printf("Timer 2 stopped.\n");
    }
  }
}

void app_main(void)
{
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello, world!");

  timer1_handle = xTimerCreate(
      "Timer1",
      pdMS_TO_TICKS(TIMER1_PERIOD_MS),
      pdTRUE,
      (void *)0,
      timer_callback);

  timer2_handle = xTimerCreate(
      "Timer2",
      pdMS_TO_TICKS(TIMER2_PERIOD_MS),
      pdTRUE,
      (void *)0,
      timer_callback);

  if (timer1_handle != NULL)
    xTimerStart(timer1_handle, 0);
  if (timer2_handle != NULL)
    xTimerStart(timer2_handle, 0);
}

void loop()
{
}
