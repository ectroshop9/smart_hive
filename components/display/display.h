#ifndef DISPLAY_H
#define DISPLAY_H

#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// تعريفات C++ نقية
extern lv_color_t *buf1;
extern lv_color_t *buf2;

esp_lcd_panel_handle_t display_init();
void display_create_ui();
void display_update_hive(const hive_data_t* data);
void display_update_ai_status(const char* status);
void display_set_selected_hive(int index);
void display_set_dropdown_options(const char* options, int selected);
void display_timer_handler();
void display_lock();
void display_unlock();
SemaphoreHandle_t display_get_mutex();

#endif
