#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_psram.h"
#include "nvs_flash.h"
#include "esp_pm.h"
#include "esp_task_wdt.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include <inttypes.h>

// إضافات الماستر لنظام الملفات
#include "esp_littlefs.h" 

#include "types.h"
#include "constants.h"
#include "display.h"
#include "joystick.h"
#include "espnow_handler.h"
#include "hive_manager.h"
#include "alarm.h"
#include "ai_engine.h"
#include "web_server.h"

static const char* TAG = "MAIN";
static QueueHandle_t ui_update_queue = nullptr;
static bool system_ready = false;

static TaskHandle_t ui_task_handle = NULL;
static TaskHandle_t ai_task_handle = NULL;

// ========== نظام التشخيص الذاتي (Diagnostics) ==========
typedef struct {
    bool memory_ok;
    bool espnow_ok;
    bool ai_engine_ok;
    bool display_ok;
    uint32_t last_sync_time;
} sys_diag_t;

static sys_diag_t master_health = {true, true, true, true, 0};

static void perform_system_diagnostics(void) {
    // 1. فحص الذاكرة
    size_t free_ram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    master_health.memory_ok = (free_ram > 10240); // التحذير إذا قل عن 10KB
    
    // 2. فحص حالة الاتصال (آخر تحديث من HiveManager)
    master_health.espnow_ok = (pdTICKS_TO_MS(xTaskGetTickCount()) - master_health.last_sync_time < 60000);

    // 3. طباعة تقرير الحالة في السجل
    ESP_LOGI(TAG, "🔍 DIAG: RAM:%s | ESPNOW:%s | AI:%s", 
             master_health.memory_ok ? "OK" : "LOW",
             master_health.espnow_ok ? "ALIVE" : "DEAD",
             master_health.ai_engine_ok ? "RUNNING" : "STOPPED");
}

// ========== تهيئة نظام ملفات LittleFS (لقراءة الـ 1500 سطر) ==========
static void init_littlefs(void) {
    ESP_LOGI(TAG, "📂 Mounting LittleFS storage...");
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/data",
        .partition_label = "storage",
        .format_if_mount_failed = true,
        .dont_mount = false
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "❌ Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "❌ Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "❌ Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    esp_littlefs_info("storage", &total, &used);
    ESP_LOGI(TAG, "✅ LittleFS Mounted: %d/%d KB used", (int)used/1024, (int)total/1024);
}

// ========== فحص وعرض معلومات الذاكرة (أصلية) ==========
static void print_memory_info(void) {
    size_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024;
    ESP_LOGI(TAG, "📊 Internal RAM: %d KB", (int)internal_free);

#ifdef CONFIG_SPIRAM
    if (esp_psram_is_initialized()) {
        size_t psram_size = esp_psram_get_size() / 1024;
        size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024;
        ESP_LOGI(TAG, "✅ PSRAM: %d KB total, %d KB free (Hardware Detected)", 
                  (int)psram_size, (int)psram_free);
    } else {
        ESP_LOGE(TAG, "❌ PSRAM Error: Enabled in menuconfig but NOT detected by Hardware!");
    }
#else
    ESP_LOGW(TAG, "⚠️ PSRAM: Disabled in configuration.");
#endif
}

// ========== تهيئة الكلب الحارس (Watchdog) ==========
static void init_watchdog(void) {
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 15000,
        .idle_core_mask = (1 << 0) | (1 << 1),
        .trigger_panic = true
    };
    ESP_ERROR_CHECK(esp_task_wdt_reconfigure(&twdt_config));
    ESP_LOGI(TAG, "🐕 Watchdog is active and guarding the hive...");
}

// ========== إدارة الطاقة (Battery Save) ==========
static void init_power_management(void) {
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 40,
        .light_sleep_enable = true 
    };
    esp_pm_configure(&pm_config);
    ESP_LOGI(TAG, "⚡ Power management enabled (40-240 MHz, light sleep)");
}

// ========== RAII Mutex Lock (أصلية) ==========
class MutexLock {
public:
    explicit MutexLock(SemaphoreHandle_t mutex) : m_mutex(mutex) {
        if (m_mutex) xSemaphoreTake(m_mutex, portMAX_DELAY);
    }
    ~MutexLock() {
        if (m_mutex) xSemaphoreGive(m_mutex);
    }
private:
    SemaphoreHandle_t m_mutex;
};

// ========== تحديث خادم الويب بالبيانات (أصلية) ==========
static void update_web_server(const hive_data_t* data) {
    if (!data) return;
    float temp_avg = (hive_get_temp_1(data) + hive_get_temp_2(data) + hive_get_temp_3(data)) / 3.0f;
    web_server_update_data(
        temp_avg, hive_get_hum(data), hive_get_weight(data), data->battery / 10.0f,
        data->sound, data->gas, data->uv, data->vibration,
        data->motion_entrance, data->motion_inside,
        75, 85,
        hive_get_temp_1(data), hive_get_temp_2(data), hive_get_temp_3(data)
    );
}

// ========== Callbacks (أصلية) ==========
static void onEspNowData(const hive_data_t* data) {
    if (!data || !system_ready) return;
    
    master_health.last_sync_time = pdTICKS_TO_MS(xTaskGetTickCount()); 
    
    hive_manager_lock_write();
    HiveManager::updateHive(*data);
    hive_manager_unlock_write();
    
    update_web_server(data);
    
    if (ui_update_queue) {
        ui_update_msg_t msg;
        msg.data = *data;
        msg.idx = HiveManager::getSelected();
        xQueueOverwrite(ui_update_queue, &msg);
    }
    
    AiEngine::setDataReady();
}

static void onJoystickMove(int delta) {
    int new_selected = HiveManager::getSelected() + delta;
    if (new_selected >= 0 && new_selected < HiveManager::getCount()) {
        HiveManager::setSelected(new_selected);
        MutexLock lock(display_get_mutex());
        display_set_selected_hive(new_selected);
        AiEngine::setDataReady();
    }
}

// ========== UI Task (أصلية) ==========
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

// ========== State Change Callback (أصلية) ==========
static void onStateChange(const HiveState& oldState, const HiveState& newState) {
    ESP_LOGI(TAG, "🎯 State: %s -> %s", stateToString(oldState), stateToString(newState));
    MutexLock lock(display_get_mutex());
    display_update_ai_status(AiEngine::getLastResult());
}

// ========== مراقبة المهام + التشخيص ==========
static void monitor_task(void *pv) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        perform_system_diagnostics(); 
        if (ui_task_handle) {
            UBaseType_t watermark = uxTaskGetStackHighWaterMark(ui_task_handle);
            ESP_LOGI(TAG, "📊 UI Task stack free: %d bytes", (int)watermark * 4);
        }
        print_memory_info();
    }
}

// ========== NVS Logging (أصلية) ==========
static void nvs_log_event(const char* event) {
    nvs_handle_t nvs;
    if (nvs_open("event_log", NVS_READWRITE, &nvs) == ESP_OK) {
        int32_t index = 0;
        nvs_get_i32(nvs, "index", &index);
        char key[16];
        snprintf(key, sizeof(key), "log_%ld", (long int)(index % 10));
        nvs_set_str(nvs, key, event);
        index++;
        nvs_set_i32(nvs, "index", index);
        nvs_commit(nvs);
        nvs_close(nvs);
    }
}

// ========== app_main (نقطة انطلاق النظام) ==========
extern "C" void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "🚀 SMART HIVE MASTER v5.3 - DIAGNOSTIC ENABLED");
    ESP_LOGI(TAG, "========================================");
    
    init_power_management();
    init_watchdog();
    print_memory_info();
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // --- تهيئة نظام الملفات قبل تشغيل السيرفر ---
    init_littlefs(); 
    
    ui_update_queue = xQueueCreate(10, sizeof(ui_update_msg_t));
    configASSERT(ui_update_queue);
    
    Alarm::init();
    HiveManager::init();
    AiEngine::init();
    AiEngine::registerStateCallback(onStateChange);
    
    joystick_init();
    joystick_register_callback(onJoystickMove);
    joystick_start_task();
    
    display_init();
    display_create_ui();
    
    espnow_handler_init();
    espnow_handler_register_callback(onEspNowData);
    
    AiEngine::start();
    
    // الآن السيرفر سيجد ملفاته في /data بفضل LittleFS
    start_web_server(); 
    
    xTaskCreatePinnedToCore(ui_update_task, "ui_task", 16384, nullptr, 3, &ui_task_handle, 1);
    xTaskCreatePinnedToCore(monitor_task, "sys_monitor", 4096, nullptr, 1, NULL, 0);
    
    nvs_log_event("System Diagnostic Mode Started");
    
    system_ready = true;
    ESP_LOGI(TAG, "✅ ALL SYSTEMS ONLINE & PROTECTED");
    
    while (1) {
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}