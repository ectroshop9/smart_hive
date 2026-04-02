#ifndef UI_H
#define UI_H

#include "lvgl.h"

// شاشات التطبيق
extern lv_obj_t *ui_main_screen;
extern lv_obj_t *ui_hives_screen;
extern lv_obj_t *ui_health_screen;
extern lv_obj_t *ui_settings_screen;

// عناصر الشاشة الرئيسية
extern lv_obj_t *ui_weight_label;
extern lv_obj_t *ui_temp_label;
extern lv_obj_t *ui_hum_label;
extern lv_obj_t *ui_batt_label;
extern lv_obj_t *ui_gas_label;
extern lv_obj_t *ui_uv_label;
extern lv_obj_t *ui_top_temp_label;
extern lv_obj_t *ui_mid_temp_label;
extern lv_obj_t *ui_bottom_temp_label;
extern lv_obj_t *ui_hive_dropdown;
extern lv_obj_t *ui_status_label;

// وظائف التهيئة
void ui_init(void);
void ui_update_data(void);

// وظائف تبديل الشاشات
void ui_show_main_screen(void);
void ui_show_hives_screen(void);
void ui_show_health_screen(void);
void ui_show_settings_screen(void);

#endif
