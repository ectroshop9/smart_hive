#include "task_monitor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>

static const char *TAG = "TASK_MONITOR";

void task_monitor_log_status() {
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    uint32_t ulTotalRunTime;

    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL) {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

        ESP_LOGI(TAG, "--------------------------------------------------");
        ESP_LOGI(TAG, "Task Name\tStatus\tPrio\tStack\tTask#");
        
        for (x = 0; x < uxArraySize; x++) {
            // استخدام PRIu32 و PRIu16 للإصلاح
            ESP_LOGI(TAG, "%-12s\t%c\t%" PRIu32 "\t%" PRIu32 "\t%" PRIu32,
                     pxTaskStatusArray[x].pcTaskName,
                     ' ', // الحالة يمكن تفصيلها لاحقاً
                     (uint32_t)pxTaskStatusArray[x].uxCurrentPriority,
                     (uint32_t)pxTaskStatusArray[x].usStackHighWaterMark,
                     (uint32_t)pxTaskStatusArray[x].xTaskNumber);
        }
        
        vPortFree(pxTaskStatusArray);
    }
}
