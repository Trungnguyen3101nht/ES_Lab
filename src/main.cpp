
#include "global.h"

// Định nghĩa loại yêu cầu
typedef enum
{
  REQ_LED = 1,
  REQ_SENSOR,
  REQ_PRINT,
  REQ_UNKNOWN
} RequestType;

// Cấu trúc request
typedef struct
{
  RequestType type;
  char data[64];
} Request;

// Hàng đợi chung
static QueueHandle_t requestQueue;

// ------------------- Reception Task -------------------
void reception_task(void *pvParameters)
{
  uint8_t data[BUF_SIZE];
  Request req;

  // Cấu hình UART
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
  uart_param_config(UART_NUM, &uart_config);
  uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

  printf("\n=== ESP32 FreeRTOS UART Command Demo ===\n");
  printf("Gõ các lệnh sau trong Serial Monitor:\n");
  printf("  - LED ON\n");
  printf("  - LED OFF\n");
  printf("  - SENSOR READ\n");
  printf("  - PRINT Hello ESP32\n\n");

  while (1)
  {
    int len = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, pdMS_TO_TICKS(1000));
    if (len > 0)
    {
      data[len] = '\0';
      printf("[Reception] Received: %s\n", data);

      // Phân loại lệnh
      if (strstr((char *)data, "LED"))
        req.type = REQ_LED;
      else if (strstr((char *)data, "SENSOR"))
        req.type = REQ_SENSOR;
      else if (strstr((char *)data, "PRINT"))
        req.type = REQ_PRINT;
      else
        req.type = REQ_UNKNOWN;

      strncpy(req.data, (char *)data, sizeof(req.data) - 1);
      req.data[sizeof(req.data) - 1] = '\0';

      if (xQueueSend(requestQueue, &req, pdMS_TO_TICKS(100)) != pdPASS)
      {
        printf("[Reception] Queue full! Dropping request.\n");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// ------------------- LED Task -------------------
void led_task(void *pvParameters)
{
  gpio_reset_pin(LED_GPIO);
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  Request req;

  while (1)
  {
    if (xQueueReceive(requestQueue, &req, portMAX_DELAY))
    {
      if (req.type == REQ_LED)
      {
        if (strstr(req.data, "ON"))
        {
          gpio_set_level(LED_GPIO, 1);
          printf("[LED Task] LED turned ON\n");
        }
        else if (strstr(req.data, "OFF"))
        {
          gpio_set_level(LED_GPIO, 0);
          printf("[LED Task] LED turned OFF\n");
        }
        else
        {
          printf("[LED Task] Unknown LED command: %s\n", req.data);
        }
      }
      else
      {
        // Không phải request cho LED → gửi lại
        xQueueSendToBack(requestQueue, &req, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
      }
    }
  }
}

// ------------------- Sensor Task -------------------
void sensor_task(void *pvParameters)
{
  Request req;
  while (1)
  {
    if (xQueueReceive(requestQueue, &req, portMAX_DELAY))
    {
      if (req.type == REQ_SENSOR)
      {
        // Giả lập giá trị cảm biến
        int fake_value = 25 + (esp_random() % 10); // 25–34°C
        printf("[Sensor Task] Sensor value: %d°C (simulated)\n", fake_value);
      }
      else
      {
        xQueueSendToBack(requestQueue, &req, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
      }
    }
  }
}

// ------------------- Print Task -------------------
void print_task(void *pvParameters)
{
  Request req;
  while (1)
  {
    if (xQueueReceive(requestQueue, &req, portMAX_DELAY))
    {
      if (req.type == REQ_PRINT)
      {
        printf("[Print Task] Message: %s\n", req.data);
      }
      else if (req.type == REQ_UNKNOWN)
      {
        printf("[Error] Unknown command: %s\n", req.data);
      }
      else
      {
        xQueueSendToBack(requestQueue, &req, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
      }
    }
  }
}

// ------------------- Main -------------------
void setup()
{
  requestQueue = xQueueCreate(REQUEST_QUEUE_LENGTH, sizeof(Request));
  if (requestQueue == NULL)
  {
    printf("Failed to create queue!\n");
    return;
  }

  xTaskCreate(reception_task, "ReceptionTask", 4096, NULL, 5, NULL);
  xTaskCreate(led_task, "LEDTask", 4096, NULL, 4, NULL);
  xTaskCreate(sensor_task, "SensorTask", 4096, NULL, 4, NULL);
  xTaskCreate(print_task, "PrintTask", 4096, NULL, 4, NULL);
}
void loop()
{
  // Không sử dụng trong FreeRTOS
}