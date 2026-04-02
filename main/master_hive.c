#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"

// تعريف الدالة المفقودة لـ ILI9341
esp_err_t esp_lcd_new_panel_ili9341(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);

static const char *TAG = "MASTER";

// تعريفات الأرجل
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   10
#define PIN_NUM_DC   11
#define PIN_NUM_RST  15
#define LCD_H_RES 320
#define LCD_V_RES 240

// Buffer LVGL
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf1[LCD_H_RES * 20];

// هيكل البيانات
typedef struct {
    int id;
    float weight;
    float temp;
    float hum;
    int battery;
    int gas;
    int uv;
    bool motion;
} hive_data_t;

#define MAX_HIVES 10
hive_data_t hives[MAX_HIVES];
int hive_count = 0;

// عناصر الواجهة الرئيسية
static lv_obj_t *main_screen;
static lv_obj_t *sidebar;
static lv_obj_t *content_area;
static lv_obj_t *weight_label;
static lv_obj_t *temp_label;
static lv_obj_t *hum_label;
static lv_obj_t *batt_label;
static lv_obj_t *gas_label;
static lv_obj_t *uv_label;
static lv_obj_t *motion_label;
static lv_obj_t *hive_name_label;
static lv_obj_t *status_label;
static lv_obj_t *health_btn, *stats_btn, *tools_btn;

// دالة الربط بين LVGL والشاشة
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

// تهيئة الشاشة
static esp_lcd_panel_handle_t init_display(void)
{
    ESP_LOGI(TAG, "Initializing SPI bus...");
    
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
    
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = PIN_NUM_CS,
        .dc_gpio_num = PIN_NUM_DC,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));
    
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    esp_lcd_panel_mirror(panel_handle, false, true);
    esp_lcd_panel_invert_color(panel_handle, true);
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    return panel_handle;
}

// إنشاء الشريط الجانبي (Sidebar)
static void create_sidebar(void)
{
    sidebar = lv_obj_create(main_screen);
    lv_obj_set_size(sidebar, 80, LCD_V_RES);
    lv_obj_set_pos(sidebar, 0, 0);
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(0x0a0a0a), 0);
    lv_obj_set_style_border_width(sidebar, 0, 0);
    lv_obj_set_style_radius(sidebar, 0, 0);
    
    // عنوان مختصر
    lv_obj_t *logo = lv_label_create(sidebar);
    lv_label_set_text(logo, "SH");
    lv_obj_set_pos(logo, 20, 20);
    lv_obj_set_style_text_color(logo, lv_color_hex(0xffc107), 0);
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_18, 0);
    
    // زر لوحة التحكم
    lv_obj_t *dash_btn = lv_btn_create(sidebar);
    lv_obj_set_size(dash_btn, 50, 50);
    lv_obj_set_pos(dash_btn, 15, 100);
    lv_obj_set_style_bg_color(dash_btn, lv_color_hex(0xffc107), 0);
    lv_obj_t *dash_icon = lv_label_create(dash_btn);
    lv_label_set_text(dash_icon, "🏠");
    lv_obj_center(dash_icon);
    
    // زر الخلايا
    lv_obj_t *hives_btn = lv_btn_create(sidebar);
    lv_obj_set_size(hives_btn, 50, 50);
    lv_obj_set_pos(hives_btn, 15, 170);
    lv_obj_set_style_bg_color(hives_btn, lv_color_hex(0x333333), 0);
    lv_obj_t *hives_icon = lv_label_create(hives_btn);
    lv_label_set_text(hives_icon, "📡");
    lv_obj_center(hives_icon);
    
    // زر الإعدادات
    lv_obj_t *set_btn = lv_btn_create(sidebar);
    lv_obj_set_size(set_btn, 50, 50);
    lv_obj_set_pos(set_btn, 15, 240);
    lv_obj_set_style_bg_color(set_btn, lv_color_hex(0x333333), 0);
    lv_obj_t *set_icon = lv_label_create(set_btn);
    lv_label_set_text(set_icon, "⚙️");
    lv_obj_center(set_icon);
}

// إنشاء منطقة المحتوى الرئيسية
static void create_content_area(void)
{
    content_area = lv_obj_create(main_screen);
    lv_obj_set_size(content_area, 230, LCD_V_RES);
    lv_obj_set_pos(content_area, 85, 0);
    lv_obj_set_style_bg_color(content_area, lv_color_hex(0x050505), 0);
    lv_obj_set_style_border_width(content_area, 0, 0);
    lv_obj_set_style_radius(content_area, 0, 0);
    
    // عنوان الخلية
    hive_name_label = lv_label_create(content_area);
    lv_label_set_text(hive_name_label, "HIVE-01");
    lv_obj_set_pos(hive_name_label, 10, 15);
    lv_obj_set_style_text_color(hive_name_label, lv_color_hex(0xffc107), 0);
    lv_obj_set_style_text_font(hive_name_label, &lv_font_montserrat_16, 0);
    
    // حالة الاتصال
    status_label = lv_label_create(content_area);
    lv_label_set_text(status_label, "🟢 ONLINE");
    lv_obj_set_pos(status_label, 140, 15);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x39ff14), 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_12, 0);
    
    // الوزن (كبير في المنتصف)
    lv_obj_t *weight_box = lv_obj_create(content_area);
    lv_obj_set_size(weight_box, 210, 70);
    lv_obj_set_pos(weight_box, 10, 50);
    lv_obj_set_style_bg_color(weight_box, lv_color_hex(0x1e1e1e), 0);
    lv_obj_set_style_border_color(weight_box, lv_color_hex(0xffc107), 0);
    lv_obj_set_style_border_width(weight_box, 1, 0);
    lv_obj_set_style_radius(weight_box, 10, 0);
    
    weight_label = lv_label_create(weight_box);
    lv_label_set_text(weight_label, "24.5");
    lv_obj_set_pos(weight_label, 50, 10);
    lv_obj_set_style_text_font(weight_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(weight_label, lv_color_hex(0xffc107), 0);
    
    lv_obj_t *weight_unit = lv_label_create(weight_box);
    lv_label_set_text(weight_unit, "kg");
    lv_obj_set_pos(weight_unit, 130, 30);
    lv_obj_set_style_text_color(weight_unit, lv_color_hex(0x888888), 0);
    
    // شبكة القياسات (2x4)
    // الصف الأول
    temp_label = lv_label_create(content_area);
    lv_label_set_text(temp_label, "🌡️ 35.2°C");
    lv_obj_set_pos(temp_label, 10, 140);
    lv_obj_set_style_text_color(temp_label, lv_color_hex(0xffffff), 0);
    
    hum_label = lv_label_create(content_area);
    lv_label_set_text(hum_label, "💧 55%");
    lv_obj_set_pos(hum_label, 120, 140);
    lv_obj_set_style_text_color(hum_label, lv_color_hex(0xffffff), 0);
    
    // الصف الثاني
    batt_label = lv_label_create(content_area);
    lv_label_set_text(batt_label, "🔋 87%");
    lv_obj_set_pos(batt_label, 10, 170);
    lv_obj_set_style_text_color(batt_label, lv_color_hex(0xffffff), 0);
    
    gas_label = lv_label_create(content_area);
    lv_label_set_text(gas_label, "💨 12%");
    lv_obj_set_pos(gas_label, 120, 170);
    lv_obj_set_style_text_color(gas_label, lv_color_hex(0xffffff), 0);
    
    // الصف الثالث
    uv_label = lv_label_create(content_area);
    lv_label_set_text(uv_label, "☀️ 120");
    lv_obj_set_pos(uv_label, 10, 200);
    lv_obj_set_style_text_color(uv_label, lv_color_hex(0xffffff), 0);
    
    motion_label = lv_label_create(content_area);
    lv_label_set_text(motion_label, "🚶 ACTIVE");
    lv_obj_set_pos(motion_label, 120, 200);
    lv_obj_set_style_text_color(motion_label, lv_color_hex(0x39ff14), 0);
    
    // أزرار الإجراءات
    health_btn = lv_btn_create(content_area);
    lv_obj_set_size(health_btn, 60, 40);
    lv_obj_set_pos(health_btn, 10, 240);
    lv_obj_set_style_bg_color(health_btn, lv_color_hex(0xffc107), 0);
    lv_obj_t *health_icon = lv_label_create(health_btn);
    lv_label_set_text(health_icon, "❤️");
    lv_obj_center(health_icon);
    
    stats_btn = lv_btn_create(content_area);
    lv_obj_set_size(stats_btn, 60, 40);
    lv_obj_set_pos(stats_btn, 80, 240);
    lv_obj_set_style_bg_color(stats_btn, lv_color_hex(0x00d4ff), 0);
    lv_obj_t *stats_icon = lv_label_create(stats_btn);
    lv_label_set_text(stats_icon, "📊");
    lv_obj_center(stats_icon);
    
    tools_btn = lv_btn_create(content_area);
    lv_obj_set_size(tools_btn, 60, 40);
    lv_obj_set_pos(tools_btn, 150, 240);
    lv_obj_set_style_bg_color(tools_btn, lv_color_hex(0x333333), 0);
    lv_obj_t *tools_icon = lv_label_create(tools_btn);
    lv_label_set_text(tools_icon, "🔧");
    lv_obj_center(tools_icon);
}

// تحديث البيانات المعروضة
static void update_ui_data(void)
{
    if (hive_count > 0) {
        lv_label_set_text_fmt(weight_label, "%.1f", hives[0].weight);
        lv_label_set_text_fmt(temp_label, "🌡️ %.1f°C", hives[0].temp);
        lv_label_set_text_fmt(hum_label, "💧 %.0f%%", hives[0].hum);
        lv_label_set_text_fmt(batt_label, "🔋 %d%%", hives[0].battery);
        lv_label_set_text_fmt(gas_label, "💨 %d%%", hives[0].gas);
        lv_label_set_text_fmt(uv_label, "☀️ %d", hives[0].uv);
        lv_label_set_text(motion_label, hives[0].motion ? "🚶 ACTIVE" : "💤 REST");
        lv_label_set_text(status_label, hives[0].motion ? "🟢 ONLINE" : "🟡 STANDBY");
    }
}

// بيانات تجريبية
static void add_test_data(void)
{
    hives[0].id = 1;
    hives[0].weight = 24.5;
    hives[0].temp = 35.2;
    hives[0].hum = 55.0;
    hives[0].battery = 87;
    hives[0].gas = 12;
    hives[0].uv = 120;
    hives[0].motion = true;
    hive_count = 1;
    update_ui_data();
}

// مهمة LVGL
static void gui_task(void *pvParameter)
{
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// المهمة الرئيسية
void app_main(void)
{
    ESP_LOGI(TAG, "Smart Hive Master starting...");
    
    // تهيئة NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // تهيئة WiFi و ESP-NOW
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_ERROR_CHECK(esp_now_init());
    
    // تهيئة الشاشة
    esp_lcd_panel_handle_t panel_handle = init_display();
    
    // تهيئة LVGL
    lv_init();
    
    // إعداد Buffer الشاشة
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, LCD_H_RES * 20);
    
    // إعداد Driver الشاشة
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);
    
    // إنشاء الواجهة
    main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x050505), 0);
    
    create_sidebar();
    create_content_area();
    
    // إضافة بيانات تجريبية
    add_test_data();
    
    // إنشاء مهمة LVGL
    xTaskCreatePinnedToCore(gui_task, "gui", 4096 * 2, NULL, 5, NULL, 1);
    
    ESP_LOGI(TAG, "System ready!");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
