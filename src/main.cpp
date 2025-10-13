#include <Arduino.h>

#define BUTTON_PIN 4

TaskHandle_t cyclicTaskHandle = NULL;
TaskHandle_t acyclicTaskHandle = NULL;

#define MyID "2014882"

void cyclicTask(void *parameter)
{
  while (1)
  {

    Serial.print("Student ID: ");
    Serial.println(MyID);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void acyclicTask(void *parameter)
{
  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Serial.println("ESP32");
  }
}

void IRAM_ATTR handleButtonInterrupt()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(acyclicTaskHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void setup()
{
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  xTaskCreate(
      cyclicTask,
      "Cyclic Task",
      2048,
      NULL,
      1,
      &cyclicTaskHandle);

  xTaskCreate(
      acyclicTask,
      "Acyclic Task",
      2048,
      NULL,
      2,
      &acyclicTaskHandle);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
}

void loop()
{
}
