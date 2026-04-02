#include "ui.h"
#include "esp_log.h"

static const char *TAG = "UI_EVENTS";

// دوال الأزرار (سيتم ربطها لاحقاً)
void ui_event_main_btn(lv_event_t *e)
{
    ESP_LOGI(TAG, "Main button clicked");
    ui_show_main_screen();
}

void ui_event_hives_btn(lv_event_t *e)
{
    ESP_LOGI(TAG, "Hives button clicked");
    ui_show_hives_screen();
}

void ui_event_health_btn(lv_event_t *e)
{
    ESP_LOGI(TAG, "Health button clicked");
    ui_show_health_screen();
}

void ui_event_settings_btn(lv_event_t *e)
{
    ESP_LOGI(TAG, "Settings button clicked");
    ui_show_settings_screen();
}
