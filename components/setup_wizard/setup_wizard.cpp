#include "setup_wizard.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "../display/display.h"
#include "../ai_engine/ai_engine.h"

static const char* TAG = "SETUP_WIZARD";
static bool setup_active = false;
static int setup_step = 0;

static const char* wilaya_list[] = {
    "1 - Adrar", "2 - Chlef", "3 - Laghouat", "4 - Oum El Bouaghi",
    "5 - Batna", "6 - Bejaia", "7 - Biskra", "8 - Bechar",
    "9 - Blida", "10 - Bouira", "11 - Tamanrasset", "12 - Tebessa",
    "13 - Tlemcen", "14 - Tiaret", "15 - Tizi Ouzou", "16 - Algiers"
};

static char brand_name[32] = "SMART_HIVE_DZ";
static char admin_password[17] = "admin123";
static int selected_wilaya = 15;

static void show_welcome(void) {
    display_lock();
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "🐝 SMART HIVE MASTER");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffc107), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    
    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "نظام خبير لإدارة المناحل");
    lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 65);
    
    lv_obj_t *inst = lv_label_create(scr);
    lv_label_set_text(inst, "اضغط الجويستيك لبدء الإعداد");
    lv_obj_align(inst, LV_ALIGN_BOTTOM_MID, 0, -30);
    
    display_unlock();
}

static void save_and_restart(void) {
    int wilaya_number = selected_wilaya + 1;
    ESP_LOGI(TAG, "💾 حفظ الإعدادات: %s, ولاية %d", brand_name, wilaya_number);
    AiEngine::configure_system(brand_name, wilaya_number, admin_password);
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
}

void setup_wizard_start(void) {
    setup_active = true;
    setup_step = 0;
    show_welcome();
}

bool setup_wizard_is_required(void) {
    return !AiEngine::is_configured();
}

bool setup_wizard_is_active(void) {
    return setup_active;
}

void setup_wizard_handle_joystick(int delta) {
    if (!setup_active) return;
    
    if (delta == 0) {
        setup_step++;
        if (setup_step >= 1) {
            save_and_restart();
        }
    }
}
