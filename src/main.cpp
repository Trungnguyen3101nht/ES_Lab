#include <Arduino.h>
// Prioritized Pre-emptive Scheduling with Time Slicing.///

// TaskHandle_t Task1, Task2;

// void task1(void *pv)
// {
//   while (1)
//   {
//     Serial.println("Task 1 running");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// void task2(void *pv)
// {
//   while (1)
//   {
//     Serial.println("Task 2 running");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// void setup()
// {
//   Serial.begin(115200);
//   xTaskCreate(task1, "Task1", 2048, NULL, 1, &Task1);
//   xTaskCreate(task2, "Task2", 2048, NULL, 1, &Task2);
// }
// void loop() {}
//-----------------------------------------------------------------------------
// Prioritized Pre-emptive Scheduling without Time Slicing. ///
// void highPriorityTask(void *pv)
// {
//   while (1)
//   {
//     Serial.println("High Priority Task running");
//     vTaskDelay(pdMS_TO_TICKS(3000));
//   }
// }

// void lowPriorityTask(void *pv)
// {
//   while (1)
//   {
//     Serial.println("Low Priority Task running");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// void setup()
// {
//   Serial.begin(115200);
//   // Task high có priority cao hơn → sẽ luôn được ưu tiên chạy
//   xTaskCreatePinnedToCore(highPriorityTask, "High", 2048, NULL, 2, NULL, 1);
//   xTaskCreatePinnedToCore(lowPriorityTask, "Low", 2048, NULL, 1, NULL, 1);
// }

// void loop() {}

//-----------------------------------------------------------------------------
// Co-operative Scheduling
void coopTask1(void *pv)
{
  while (1)
  {
    Serial.println("Task 1 run...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void coopTask2(void *pv)
{
  while (1)
  {
    Serial.println("Task 2 run...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  xTaskCreate(coopTask1, "Coop1", 2048, NULL, 1, NULL);
  xTaskCreate(coopTask2, "Coop2", 2048, NULL, 1, NULL);
}
void loop() {}
