#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
// #include "esp_psram.h"  // تم تعليقها - PSRAM غير مستخدمة حالياً
#include "nvs_flash.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include <inttypes.h>

#include "types.h"
#include "constants.h"
#include "display.h"
#include "joystick.h"
#include "espnow_handler.h"
#include "hive_manager.h"
#include "alarm.h"
#include "ai_engine.h"

static const char* TAG = "MAIN";
static QueueHandle_t ui_update_queue = nullptr;
static bool system_ready = false;

// ========== configASSERT ==========
#ifndef configASSERT
#define configASSERT(x) if ((x) == 0) { \
    ESP_LOGE("FREERTOS", "Assert failed! %s:%d", __FILE__, __LINE__); \
    while (1); \
}
#endif

// ========== RAII Mutex Lock ==========
class MutexLock {
public:
    explicit MutexLock(SemaphoreHandle_t mutex) : m_mutex(mutex) {
        if (m_mutex) xSemaphoreTake(m_mutex, portMAX_DELAY);
    }
    ~MutexLock() {
        if (m_mutex) xSemaphoreGive(m_mutex);
    }
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;
private:
    SemaphoreHandle_t m_mutex;
};

// ========== Callbacks ==========
static void onEspNowData(const hive_data_t* data) {
    if (!data || !system_ready) return;
    
    HiveManager::updateHive(*data);
    int selected = HiveManager::getSelected();
    
    if (ui_update_queue) {
        ui_update_msg_t msg;
        msg.data = *data;
        msg.idx = selected;
        xQueueOverwrite(ui_update_queue, &msg);
    }
    
    AiEngine::setDataReady();
}

static void onJoystickMove(int delta) {
    int selected = HiveManager::getSelected();
    int count = HiveManager::getCount();
    int new_selected = selected + delta;
    
    if (new_selected >= 0 && new_selected < count) {
        HiveManager::setSelected(new_selected);
        MutexLock lock(display_get_mutex());
        display_set_selected_hive(new_selected);
        AiEngine::setDataReady();
    }
}

// ========== UI Task ==========
static void ui_update_task(void *pv) {
    ui_update_msg_t msg;
    
    while (1) {
        if (xQueueReceive(ui_update_queue, &msg, pdMS_TO_TICKS(10)) == pdTRUE) {
            MutexLock lock(display_get_mutex());
            display_update_hive(&msg.data);
        }
        
        MutexLock lock(display_get_mutex());
        display_update_ai_status(AiEngine::getLastResult());
        display_timer_handler();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ========== State Change Callback ==========
static void onStateChange(const HiveState& oldState, const HiveState& newState) {
    ESP_LOGI(TAG, "🎯 State Transition: %s → %s", 
             stateToString(oldState), 
             stateToString(newState));
    
    MutexLock lock(display_get_mutex());
    display_update_ai_status(AiEngine::getLastResult());
}

// ========== app_main ==========
extern "C" void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "SMART HIVE MASTER v4.0 - Industrial");
    ESP_LOGI(TAG, "========================================");
    
    // 1. PSRAM غير مستخدمة حالياً (تم تعليقها لتجنب مشاكل الربط)
    // esp_psram_init();
    // ESP_LOGI(TAG, "PSRAM: %" PRIu32 " KB", (uint32_t)(esp_psram_get_size() / 1024));
    ESP_LOGI(TAG, "Running without PSRAM (Internal RAM only)");
    
    // 2. تهيئة NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 3. تهيئة UI Queue
    ui_update_queue = xQueueCreate(3, sizeof(ui_update_msg_t));
    configASSERT(ui_update_queue);
    
    // 4. تهيئة المكونات
    Alarm::init();
    HiveManager::init();
    AiEngine::init();
    AiEngine::registerStateCallback(onStateChange);
    
    // 5. تهيئة المحيطات
    joystick_init();
    joystick_register_callback(onJoystickMove);
    joystick_start_task();
    
    // 6. تهيئة الشاشة
    display_init();
    display_create_ui();
    
    // 7. تهيئة ESP-NOW
    espnow_handler_init();
    espnow_handler_register_callback(onEspNowData);
    
    // 8. بدء AI Engine
    AiEngine::start();
    
    // 9. بدء UI Task
    xTaskCreatePinnedToCore(ui_update_task, "ui_updater", 4096, nullptr, 3, nullptr, 1);
    
    system_ready = true;
    ESP_LOGI(TAG, "✅ All Systems Online!");
    
    // 10. الحلقة الرئيسية
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
