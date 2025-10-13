#include <Arduino.h>

// Prioritized Pre-emptive Scheduling with Time Slicing.///
//  void setup()
//  {
//    Serial.begin(115200);
//    Serial.println("Hello, world!");
//  }

// void loop()
// {
//   Serial.println("Hello, world!");
// }
// #include <Arduino.h>

// TaskHandle_t Task1, Task2;

// void task1(void *pv) {
//   while (1) {
//     Serial.println("Task 1 running");
//     vTaskDelay(pdMS_TO_TICKS(100));
//   }
// }

// void task2(void *pv) {
//   while (1) {
//     Serial.println("Task 2 running");
//     vTaskDelay(pdMS_TO_TICKS(100));
//   }
// }

// void setup() {
//   Serial.begin(115200);

//   // Both tasks same priority => time slicing occurs
//   xTaskCreate(task1, "Task1", 2048, NULL, 1, &Task1);
//   xTaskCreate(task2, "Task2", 2048, NULL, 1, &Task2);
// }

// Prioritized Pre-emptive Scheduling without Time Slicing. ///
// #define configUSE_TIME_SLICING 0

// void highPriorityTask(void *pv) {
//   while (1) {
//     Serial.println("High Priority Task running");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// void lowPriorityTask(void *pv) {
//   while (1) {
//     Serial.println("Low Priority Task running");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// void setup() {
//   Serial.begin(115200);

//   xTaskCreate(highPriorityTask, "High", 2048, NULL, 2, NULL);
//   xTaskCreate(lowPriorityTask, "Low", 2048, NULL, 1, NULL);
// }

// void loop()
// {
//   // Empty loop
// }

// Co-operative Scheduling
void coopTask1(void *pv)
{
  while (1)
  {
    Serial.println("Task 1 doing work...");
    taskYIELD();
  }
}

void coopTask2(void *pv)
{
  while (1)
  {
    Serial.println("Task 2 doing work...");
    taskYIELD();
  }
}

void setup()
{
  Serial.begin(115200);
  xTaskCreate(coopTask1, "Coop1", 2048, NULL, 1, NULL);
  xTaskCreate(coopTask2, "Coop2", 2048, NULL, 1, NULL);
}

// extra exercise

volatile uint32_t idleCounter = 0;
uint32_t lastMillis = 0;

extern "C" void vApplicationIdleHook(void)
{
  idleCounter++;
}

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
  uint32_t now = millis();
  static uint32_t lastCount = 0;

  uint32_t deltaCount = idleCounter - lastCount;
  lastCount = idleCounter;

  float cpuIdlePercent = (deltaCount / 10000.0) * 100;
  float cpuUsage = 100.0 - cpuIdlePercent;

  Serial.printf("CPU Usage: %.2f%%\n", cpuUsage);
}
