#include <Arduino.h>

#define BUTTON_PIN 0

TaskHandle_t cyclicTaskHandle = NULL;
TaskHandle_t acyclicTaskHandle = NULL;

#define MyID "2014882"

void cyclicTask(void *parameter)
{
  while (1)
  {

    Serial.println("Student ID: " + String(MyID));
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
// vì cần sử lí ngay lập tức mà nếu lưu tròn flash memory thì có thể bị miss trong một số tính huống(như truy cập SPIflash) nên lưu trong RAM sẽ an toàn và thực thi ổn định hơn
void IRAM_ATTR handleButtonInterrupt()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  // phân biệt xem task nào có độ ưu tiên cao hơn để thực hiện sau khi thực hiện ngắt
  vTaskNotifyGiveFromISR(acyclicTaskHandle, &xHigherPriorityTaskWoken);
  // gửi tín hiệu ngắn đến task acyclicTask đê nó thực thi, nếu task có độ ưu tiên cao hơn task đang thực thi, gán pdTRUE
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  // đảm bảo task có độ ưu tiên cao hơn được thực thi ngay lập tức sau khi ngắt xảy ra
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
  // giúp  nhận nút nhấn ngay mà ko cần phải kiểm tra liên tục trong loop
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
  /*
  - digitalPinToInterrupt(BUTTON_PIN) → Lấy số ngắt tương ứng với chân đó.
  - handleButtonInterrupt → Hàm sẽ được gọi ngay lập tức khi ngắt xảy ra.
  - FALLING → Kiểu ngắt: xảy ra khi tín hiệu chuyển từ mức cao xuống mức thấp (nút được nhấn trong trường hợp dùng INPUT_PULLUP).
  */
}

void loop()
{
}
