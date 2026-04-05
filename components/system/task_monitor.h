#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

void task_monitor_register(TaskHandle_t handle, const char* name, uint32_t stack_size);
void task_monitor_update(void);
void task_monitor_print_all(void);

#ifdef __cplusplus
}
#endif
