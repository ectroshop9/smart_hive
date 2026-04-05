#ifndef DISPLAY_H
#define DISPLAY_H

#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "../utils/types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// متغيرات LVGL العامة
extern lv_disp_draw_buf_t disp_buf;
extern lv_color_t buf1[];
extern lv_color_t buf2[];
extern lv_obj_t *weight_box, *weight_label, *temp_label_1, *temp_label_2, *temp_label_3;
extern lv_obj_t *hum_label, *batt_label, *gas_label, *uv_label;
extern lv_obj_t *motion_entrance_label, *motion_inside_label, *sound_label, *vibration_label;
extern lv_obj_t *status_label, *hive_selector, *ai_label;
extern SemaphoreHandle_t lvgl_mutex;

// دوال العرض
esp_lcd_panel_handle_t display_init(void);
void display_create_ui(void);
void display_update_hive(const hive_data_t* data);
void display_update_ai_status(const char* status);
void display_set_selected_hive(int index);
void display_set_dropdown_options(const char* options, int selected);
void display_lock(void);
void display_unlock(void);
void display_timer_handler(void);

// دوال LVGL
void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
lv_color_t get_value_color(float value, float min_normal, float max_normal, bool inverse);
void nav_btn_cb(lv_event_t *e);
void hive_selector_cb(lv_event_t *e);
static inline SemaphoreHandle_t display_get_mutex(void) { return lvgl_mutex; }

#ifdef __cplusplus
}
#endif

#endif
