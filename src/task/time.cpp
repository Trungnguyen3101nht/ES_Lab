#include "time.h"

struct tm currentTime;
SemaphoreHandle_t timeMutex;
void TaskTime(void *pvParameters)
{
    for (;;)
    {
        struct tm timeinfo;

        // Lấy giờ từ hệ thống (đã sync với SNTP)
        if (getLocalTime(&timeinfo))
        {
            xSemaphoreTake(timeMutex, portMAX_DELAY);
            currentTime = timeinfo; // ghi vào biến global
            xSemaphoreGive(timeMutex);

            // Serial.printf("⏳ Time: %02d:%02d:%02d\n",
            //               timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // cập nhật mỗi giây
    }
}

void Time_Init()
{
    timeMutex = xSemaphoreCreateMutex();

    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    xTaskCreatePinnedToCore(
        TaskTime,   // Hàm thực thi của task
        "TaskTime", // Tên của task
        4096,       // Kích thước stack (bytes)
        NULL,       // Tham số truyền vào
        2,          // Độ ưu tiên
        NULL,       // Handle của task
        1           // Core chạy task (1 cho core 1)
    );
}