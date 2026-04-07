#include "display.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_ili9341.h"
#include "driver/spi_master.h"
#include "../utils/constants.h"
#include <inttypes.h>

static const char* TAG = "DISPLAY";

// تعريف المتغيرات العامة
lv_disp_draw_buf_t disp_buf;
lv_color_t buf1[LCD_H_RES * 60];
lv_color_t buf2[LCD_H_RES * 60];
lv_obj_t *weight_box = nullptr;
lv_obj_t *weight_label = nullptr;
lv_obj_t *temp_label_1 = nullptr;
lv_obj_t *temp_label_2 = nullptr;
lv_obj_t *temp_label_3 = nullptr;
lv_obj_t *hum_label = nullptr;
lv_obj_t *batt_label = nullptr;
lv_obj_t *gas_label = nullptr;
lv_obj_t *uv_label = nullptr;
lv_obj_t *motion_entrance_label = nullptr;
lv_obj_t *motion_inside_label = nullptr;
lv_obj_t *sound_label = nullptr;
lv_obj_t *vibration_label = nullptr;
lv_obj_t *status_label = nullptr;
lv_obj_t *hive_selector = nullptr;
lv_obj_t *ai_label = nullptr;
SemaphoreHandle_t lvgl_mutex = nullptr;

void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

lv_color_t get_value_color(float value, float min_normal, float max_normal, bool inverse) {
    if (inverse) {
        if (value < min_normal) return lv_color_hex(0xFF0000);
        if (value < max_normal) return lv_color_hex(0xFFA500);
        return lv_color_hex(0xFFFFFF);
    } else {
        if (value > max_normal) return lv_color_hex(0xFF0000);
        if (value > min_normal) return lv_color_hex(0xFFA500);
        return lv_color_hex(0xFFFFFF);
    }
}

void nav_btn_cb(lv_event_t *e) {
    const char *dir = (const char *)lv_event_get_user_data(e);
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Nav button clicked: %s", dir);
    }
}

void hive_selector_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        int new_idx = lv_dropdown_get_selected(lv_event_get_target(e));
        ESP_LOGI(TAG, "Hive selected: %d", new_idx);
    }
}

esp_lcd_panel_handle_t display_init(void) {
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = PIN_NUM_MOSI;
    bus_cfg.miso_io_num = -1;
    bus_cfg.sclk_io_num = PIN_NUM_CLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t);

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = nullptr;
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.cs_gpio_num = PIN_NUM_CS;
    io_config.dc_gpio_num = PIN_NUM_DC;
    io_config.spi_mode = 0;
    io_config.pclk_hz = 40 * 1000 * 1000;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;

    // التصحيح: تحويل النوع Explicit Cast
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = nullptr;
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = PIN_NUM_RST;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
    panel_config.data_endian = LCD_RGB_DATA_ENDIAN_BIG; // تصحيح لـ v5.3
    panel_config.bits_per_pixel = 16;

    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Display initialized");
    return panel_handle;
}

void display_create_ui(void) {
    lvgl_mutex = xSemaphoreCreateMutex();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    hive_selector = lv_dropdown_create(scr);
    lv_obj_set_size(hive_selector, 110, 35);
    lv_obj_set_pos(hive_selector, 80, 10);
    lv_obj_add_event_cb(hive_selector, hive_selector_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_t *btn_prev = lv_btn_create(scr);
    lv_obj_set_size(btn_prev, 40, 35);
    lv_obj_set_pos(btn_prev, 30, 10);
    lv_obj_add_event_cb(btn_prev, nav_btn_cb, LV_EVENT_CLICKED, (char*)"prev");
    lv_obj_set_style_bg_color(btn_prev, lv_color_hex(0x333333), 0);
    lv_label_set_text(lv_label_create(btn_prev), "<");

    lv_obj_t *btn_next = lv_btn_create(scr);
    lv_obj_set_size(btn_next, 40, 35);
    lv_obj_set_pos(btn_next, 200, 10);
    lv_obj_add_event_cb(btn_next, nav_btn_cb, LV_EVENT_CLICKED, (char*)"next");
    lv_obj_set_style_bg_color(btn_next, lv_color_hex(0x333333), 0);
    lv_label_set_text(lv_label_create(btn_next), ">");

    weight_box = lv_obj_create(scr);
    lv_obj_set_size(weight_box, 240, 90);
    lv_obj_align(weight_box, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_set_style_radius(weight_box, 8, 0);
    lv_obj_set_style_bg_color(weight_box, lv_color_hex(0x1e1e1e), 0);
    lv_obj_set_style_border_width(weight_box, 2, 0);
    lv_obj_set_style_border_color(weight_box, lv_color_hex(0xffc107), 0);

    weight_label = lv_label_create(weight_box);
    lv_obj_set_style_text_font(weight_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(weight_label, lv_color_hex(0xffc107), 0);
    lv_obj_center(weight_label);

    temp_label_1 = lv_label_create(scr); lv_obj_set_pos(temp_label_1, 5, 150);
    temp_label_2 = lv_label_create(scr); lv_obj_set_pos(temp_label_2, 5, 170);
    temp_label_3 = lv_label_create(scr); lv_obj_set_pos(temp_label_3, 5, 190);
    hum_label = lv_label_create(scr); lv_obj_set_pos(hum_label, 120, 150);
    batt_label = lv_label_create(scr); lv_obj_set_pos(batt_label, 120, 170);
    gas_label = lv_label_create(scr); lv_obj_set_pos(gas_label, 120, 190);
    uv_label = lv_label_create(scr); lv_obj_set_pos(uv_label, 230, 150);
    motion_entrance_label = lv_label_create(scr); lv_obj_set_pos(motion_entrance_label, 230, 170);
    motion_inside_label = lv_label_create(scr); lv_obj_set_pos(motion_inside_label, 230, 190);
    sound_label = lv_label_create(scr); lv_obj_set_pos(sound_label, 5, 210);
    vibration_label = lv_label_create(scr); lv_obj_set_pos(vibration_label, 120, 210);
    status_label = lv_label_create(scr);
    lv_obj_align(status_label, LV_ALIGN_TOP_RIGHT, -10, 60);
    ai_label = lv_label_create(scr);
    lv_obj_set_pos(ai_label, 10, 230);

    lv_obj_t *joystick_hint = lv_label_create(scr);
    lv_label_set_text(joystick_hint, "LEFT/RIGHT");
    lv_obj_set_style_text_color(joystick_hint, lv_color_hex(0x888888), 0);
    lv_obj_set_pos(joystick_hint, 10, 5);

    ESP_LOGI(TAG, "UI created");
}

void display_update_hive(const hive_data_t* data) {
    if (!data || !lvgl_mutex) return;

    if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        char weight_str[20];
        snprintf(weight_str, sizeof(weight_str), "%.1f kg", hive_get_weight(data));
        lv_label_set_text(weight_label, weight_str);

        lv_label_set_text_fmt(temp_label_1, "T1: %.1fC", hive_get_temp_1(data));
        lv_label_set_text_fmt(temp_label_2, "T2: %.1fC", hive_get_temp_2(data));
        lv_label_set_text_fmt(temp_label_3, "T3: %.1fC", hive_get_temp_3(data));
        lv_label_set_text_fmt(hum_label, "Hum %.0f%%", hive_get_hum(data));
        lv_label_set_text_fmt(batt_label, "Bat %d", data->battery);
        lv_label_set_text_fmt(gas_label, "Gas %d", data->gas);
        lv_label_set_text_fmt(uv_label, "UV %d", data->uv);
        lv_label_set_text(motion_entrance_label, data->motion_entrance ? "ENT ACTIVE" : "ENT REST");
        lv_label_set_text(motion_inside_label, data->motion_inside ? "IN ACTIVE" : "IN REST");
        lv_label_set_text_fmt(sound_label, "SND %d", data->sound);
        lv_label_set_text_fmt(vibration_label, "VIB %d", data->vibration);

        lv_obj_set_style_text_color(temp_label_1, get_value_color(hive_get_temp_1(data), TEMP_WARNING, TEMP_CRITICAL, false), 0);
        lv_obj_set_style_text_color(temp_label_2, get_value_color(hive_get_temp_2(data), TEMP_WARNING, TEMP_CRITICAL, false), 0);
        lv_obj_set_style_text_color(temp_label_3, get_value_color(hive_get_temp_3(data), TEMP_WARNING, TEMP_CRITICAL, false), 0);
        lv_obj_set_style_text_color(hum_label, get_value_color(hive_get_hum(data), HUM_WARNING, HUM_CRITICAL, true), 0);
        lv_obj_set_style_text_color(batt_label, get_value_color((float)data->battery, BATTERY_WARNING, BATTERY_CRITICAL, true), 0);
        lv_obj_set_style_text_color(gas_label, get_value_color((float)data->gas, GAS_WARNING, GAS_CRITICAL, false), 0);

        xSemaphoreGive(lvgl_mutex);
    }
}

void display_update_ai_status(const char* status) {
    if (!status || !lvgl_mutex) return;
    if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        lv_label_set_text(ai_label, status);
        xSemaphoreGive(lvgl_mutex);
    }
}

void display_set_selected_hive(int index) {
    if (!hive_selector || !lvgl_mutex) return;
    if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        lv_dropdown_set_selected(hive_selector, (uint16_t)index);
        xSemaphoreGive(lvgl_mutex);
    }
}

void display_set_dropdown_options(const char* options, int selected) {
    if (!hive_selector || !options || !lvgl_mutex) return;
    if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        lv_dropdown_set_options(hive_selector, options);
        lv_dropdown_set_selected(hive_selector, (uint16_t)selected);
        xSemaphoreGive(lvgl_mutex);
    }
}

void display_lock(void) {
    if (lvgl_mutex) xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
}

void display_unlock(void) {
    if (lvgl_mutex) xSemaphoreGive(lvgl_mutex);
}

void display_timer_handler(void) {
    lv_timer_handler();
}
