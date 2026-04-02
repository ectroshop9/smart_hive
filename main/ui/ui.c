#include "ui.h"
#include "esp_log.h"

static const char *TAG = "UI";

// تعريف الشاشات
lv_obj_t *ui_main_screen = NULL;
lv_obj_t *ui_hives_screen = NULL;
lv_obj_t *ui_health_screen = NULL;
lv_obj_t *ui_settings_screen = NULL;

// عناصر الشاشة الرئيسية
lv_obj_t *ui_weight_label = NULL;
lv_obj_t *ui_temp_label = NULL;
lv_obj_t *ui_hum_label = NULL;
lv_obj_t *ui_batt_label = NULL;
lv_obj_t *ui_gas_label = NULL;
lv_obj_t *ui_uv_label = NULL;
lv_obj_t *ui_top_temp_label = NULL;
lv_obj_t *ui_mid_temp_label = NULL;
lv_obj_t *ui_bottom_temp_label = NULL;
lv_obj_t *ui_hive_dropdown = NULL;
lv_obj_t *ui_status_label = NULL;

// قيم وهمية (سيتم استبدالها بالبيانات الحقيقية لاحقاً)
static float current_weight = 24.5;
static float current_temp = 35.2;
static float current_hum = 55.0;
static float current_batt = 4.1;
static int current_gas = 12;
static int current_uv = 120;
static float top_temp = 34.2;
static float mid_temp = 35.5;
static float bottom_temp = 33.8;

// ============= دوال مساعدة =============
static void create_card(lv_obj_t *parent, const char *title, int x, int y, int w, int h)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1e1e1e), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xffc107), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 15, 0);
    
    lv_obj_t *title_label = lv_label_create(card);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffc107), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(title_label, 10, 10);
}

static void create_metric_box(lv_obj_t *parent, const char *icon, const char *unit, int x, int y, int w, int h, lv_obj_t **value_label)
{
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_size(box, w, h);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x111111), 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_radius(box, 10, 0);
    
    lv_obj_t *icon_label = lv_label_create(box);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_pos(icon_label, 5, 5);
    lv_obj_set_style_text_color(icon_label, lv_color_hex(0xffc107), 0);
    
    *value_label = lv_label_create(box);
    lv_label_set_text(*value_label, "--");
    lv_obj_set_style_text_font(*value_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(*value_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_pos(*value_label, 5, 35);
    
    lv_obj_t *unit_label = lv_label_create(box);
    lv_label_set_text(unit_label, unit);
    lv_obj_set_pos(unit_label, 50, 42);
    lv_obj_set_style_text_color(unit_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(unit_label, &lv_font_montserrat_12, 0);
}

// ============= الشاشة الرئيسية =============
static void create_main_screen(void)
{
    ui_main_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_main_screen, lv_color_hex(0x050505), 0);
    
    // الهيدر (الشريط العلوي)
    lv_obj_t *header = lv_obj_create(ui_main_screen);
    lv_obj_set_size(header, 320, 40);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xffc107), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "SMART HIVE PRO");
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_center(title);
    
    // زر تبديل الخلية
    ui_hive_dropdown = lv_dropdown_create(ui_main_screen);
    lv_dropdown_set_options(ui_hive_dropdown, "HIVE-01\nHIVE-02\nHIVE-03\nHIVE-04");
    lv_obj_set_pos(ui_hive_dropdown, 10, 55);
    lv_obj_set_size(ui_hive_dropdown, 100, 30);
    lv_obj_set_style_bg_color(ui_hive_dropdown, lv_color_hex(0x111111), 0);
    lv_obj_set_style_text_color(ui_hive_dropdown, lv_color_hex(0xffc107), 0);
    
    // حالة الاتصال
    ui_status_label = lv_label_create(ui_main_screen);
    lv_label_set_text(ui_status_label, "🟢 ONLINE");
    lv_obj_set_pos(ui_status_label, 250, 60);
    lv_obj_set_style_text_color(ui_status_label, lv_color_hex(0x39ff14), 0);
    
    // الوزن (مركز الشاشة)
    lv_obj_t *weight_box = lv_obj_create(ui_main_screen);
    lv_obj_set_size(weight_box, 280, 80);
    lv_obj_set_pos(weight_box, 20, 100);
    lv_obj_set_style_bg_color(weight_box, lv_color_hex(0x111111), 0);
    lv_obj_set_style_border_width(weight_box, 0, 0);
    lv_obj_set_style_radius(weight_box, 15, 0);
    
    ui_weight_label = lv_label_create(weight_box);
    lv_label_set_text(ui_weight_label, "24.5");
    lv_obj_set_style_text_font(ui_weight_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(ui_weight_label, lv_color_hex(0xffc107), 0);
    lv_obj_set_pos(ui_weight_label, 80, 15);
    
    lv_obj_t *weight_unit = lv_label_create(weight_box);
    lv_label_set_text(weight_unit, "kg");
    lv_obj_set_pos(weight_unit, 170, 30);
    lv_obj_set_style_text_color(weight_unit, lv_color_hex(0x888888), 0);
    
    // شبكة القياسات (4x2)
    create_metric_box(ui_main_screen, "🌡️", "°C", 20, 200, 70, 70, &ui_temp_label);
    create_metric_box(ui_main_screen, "💧", "%", 120, 200, 70, 70, &ui_hum_label);
    create_metric_box(ui_main_screen, "🔋", "V", 220, 200, 70, 70, &ui_batt_label);
    create_metric_box(ui_main_screen, "💨", "%", 20, 280, 70, 70, &ui_gas_label);
    create_metric_box(ui_main_screen, "☀️", "%", 120, 280, 70, 70, &ui_uv_label);
    
    // حساسات DS18B20 (3 حساسات)
    lv_obj_t *temp_section = lv_label_create(ui_main_screen);
    lv_label_set_text(temp_section, "📊 حساسات الحرارة");
    lv_obj_set_pos(temp_section, 20, 370);
    lv_obj_set_style_text_color(temp_section, lv_color_hex(0xffc107), 0);
    
    create_metric_box(ui_main_screen, "⬆️ أعلى", "°C", 20, 400, 85, 65, &ui_top_temp_label);
    create_metric_box(ui_main_screen, "👑 وسط", "°C", 115, 400, 85, 65, &ui_mid_temp_label);
    create_metric_box(ui_main_screen, "⬇️ أسفل", "°C", 210, 400, 85, 65, &ui_bottom_temp_label);
    
    // أزرار التنقل السفلية
    lv_obj_t *btn_main = lv_btn_create(ui_main_screen);
    lv_obj_set_pos(btn_main, 10, 490);
    lv_obj_set_size(btn_main, 70, 40);
    lv_obj_set_style_bg_color(btn_main, lv_color_hex(0xffc107), 0);
    lv_obj_t *btn_main_label = lv_label_create(btn_main);
    lv_label_set_text(btn_main_label, "🏠");
    lv_obj_center(btn_main_label);
    
    lv_obj_t *btn_hives = lv_btn_create(ui_main_screen);
    lv_obj_set_pos(btn_hives, 90, 490);
    lv_obj_set_size(btn_hives, 70, 40);
    lv_obj_set_style_bg_color(btn_hives, lv_color_hex(0x333333), 0);
    lv_obj_t *btn_hives_label = lv_label_create(btn_hives);
    lv_label_set_text(btn_hives_label, "📡");
    lv_obj_center(btn_hives_label);
    
    lv_obj_t *btn_health = lv_btn_create(ui_main_screen);
    lv_obj_set_pos(btn_health, 170, 490);
    lv_obj_set_size(btn_health, 70, 40);
    lv_obj_set_style_bg_color(btn_health, lv_color_hex(0x333333), 0);
    lv_obj_t *btn_health_label = lv_label_create(btn_health);
    lv_label_set_text(btn_health_label, "❤️");
    lv_obj_center(btn_health_label);
    
    lv_obj_t *btn_settings = lv_btn_create(ui_main_screen);
    lv_obj_set_pos(btn_settings, 250, 490);
    lv_obj_set_size(btn_settings, 70, 40);
    lv_obj_set_style_bg_color(btn_settings, lv_color_hex(0x333333), 0);
    lv_obj_t *btn_settings_label = lv_label_create(btn_settings);
    lv_label_set_text(btn_settings_label, "⚙️");
    lv_obj_center(btn_settings_label);
}

// ============= تحديث البيانات =============
void ui_update_data(void)
{
    if (ui_weight_label) {
        lv_label_set_text_fmt(ui_weight_label, "%.1f", current_weight);
    }
    if (ui_temp_label) {
        lv_label_set_text_fmt(ui_temp_label, "%.1f", current_temp);
    }
    if (ui_hum_label) {
        lv_label_set_text_fmt(ui_hum_label, "%.0f", current_hum);
    }
    if (ui_batt_label) {
        lv_label_set_text_fmt(ui_batt_label, "%.1f", current_batt);
    }
    if (ui_gas_label) {
        lv_label_set_text_fmt(ui_gas_label, "%d", current_gas);
    }
    if (ui_uv_label) {
        lv_label_set_text_fmt(ui_uv_label, "%d", current_uv);
    }
    if (ui_top_temp_label) {
        lv_label_set_text_fmt(ui_top_temp_label, "%.1f", top_temp);
    }
    if (ui_mid_temp_label) {
        lv_label_set_text_fmt(ui_mid_temp_label, "%.1f", mid_temp);
    }
    if (ui_bottom_temp_label) {
        lv_label_set_text_fmt(ui_bottom_temp_label, "%.1f", bottom_temp);
    }
}

// ============= دوال تبديل الشاشات =============
void ui_show_main_screen(void)
{
    if (ui_main_screen) {
        lv_scr_load(ui_main_screen);
    }
}

void ui_show_hives_screen(void)
{
    ESP_LOGI(TAG, "Show hives screen (to be implemented)");
}

void ui_show_health_screen(void)
{
    ESP_LOGI(TAG, "Show health screen (to be implemented)");
}

void ui_show_settings_screen(void)
{
    ESP_LOGI(TAG, "Show settings screen (to be implemented)");
}

// ============= التهيئة الرئيسية =============
void ui_init(void)
{
    ESP_LOGI(TAG, "Initializing UI...");
    
    create_main_screen();
    ui_show_main_screen();
    ui_update_data();
    
    ESP_LOGI(TAG, "UI initialized successfully");
}
