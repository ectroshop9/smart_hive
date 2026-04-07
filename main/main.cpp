#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_psram.h"
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
#include "web_server.h"

static const char* TAG = "MAIN";
static QueueHandle_t ui_update_queue = nullptr;
static bool system_ready = false;

// تعريف مقابض المهام في البداية لتجنب تناقض extern/static
static TaskHandle_t ui_task_handle = NULL;
static TaskHandle_t ai_task_handle = NULL;

// إحصائيات المهام
static void monitor_task_stack(TaskHandle_t task, const char* name) {
    if (task == NULL) return;
    UBaseType_t watermark = uxTaskGetStackHighWaterMark(task);
    ESP_LOGI(TAG, "📊 Task '%s' stack free: %d bytes", name, (int)watermark * 4);
    if (watermark < 500) {
        ESP_LOGW(TAG, "⚠️ Task '%s' stack low!", name);
    }
}

// ========== RAII Mutex Lock ==========
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

// ========== تحديث خادم الويب بالبيانات ==========
static void update_web_server(const hive_data_t* data) {
    if (!data) return;
    float temp_avg = (hive_get_temp_1(data) + hive_get_temp_2(data) + hive_get_temp_3(data)) / 3.0f;
    web_server_update_data(
        temp_avg, hive_get_hum(data), hive_get_weight(data), data->battery / 10.0f,
        data->sound, data->gas, data->uv, data->vibration,
        data->motion_entrance, data->motion_inside,
        75, 85, // Colony strength & health score
        hive_get_temp_1(data), hive_get_temp_2(data), hive_get_temp_3(data)
    );
}

// ========== Callbacks ==========
static void onEspNowData(const hive_data_t* data) {
    if (!data || !system_ready) return;
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

// ========== Tasks ==========
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

static void monitor_task(void *pv) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        monitor_task_stack(ui_task_handle, "ui_updater");
        monitor_task_stack(ai_task_handle, "ai_engine");
        ESP_LOGI(TAG, "💾 Heap: %d | PSRAM: %d", 
                 (int)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                 (int)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    }
}

static void onStateChange(const HiveState& oldState, const HiveState& newState) {
    ESP_LOGI(TAG, "🎯 State: %s -> %s", stateToString(oldState), stateToString(newState));
    MutexLock lock(display_get_mutex());
    display_update_ai_status(AiEngine::getLastResult());
}

static void nvs_log_event(const char* event) {
    nvs_handle_t nvs;
    if (nvs_open("event_log", NVS_READWRITE, &nvs) == ESP_OK) {
        int32_t index = 0; // تصحيح نوع البيانات هنا
        nvs_get_i32(nvs, "index", &index);
        char key[16];
        snprintf(key, sizeof(key), "log_%ld", (long int)(index % 10));
        nvs_set_str(nvs, key, event);
        nvs_set_i32(nvs, "index", ++index);
        nvs_commit(nvs);
        nvs_close(nvs);
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "🚀 SMART HIVE STARTING...");
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ui_update_queue = xQueueCreate(3, sizeof(ui_update_msg_t));
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
    start_web_server();
    
    xTaskCreatePinnedToCore(ui_update_task, "ui_updater", 8192, nullptr, 3, &ui_task_handle, 1);
    xTaskCreatePinnedToCore(monitor_task, "monitor", 4096, nullptr, 1, NULL, 1);
    
    nvs_log_event("System started");
    system_ready = true;
    ESP_LOGI(TAG, "✅ ALL SYSTEMS ONLINE");

    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
