#include "espnow_handler.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "../utils/constants.h"
#include "../utils/types.h"
#include <cstring>

static const char* TAG = "ESPNOW";
static espnow_data_callback_t data_callback = nullptr;
static QueueHandle_t esp_now_queue = NULL;
#define ESPNOW_QUEUE_SIZE 50

// دالة الاستقبال من ISR - ترسل للطابور فقط
static void on_data_receive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len != sizeof(hive_data_t)) return;

    hive_data_t received;
    memcpy(&received, data, sizeof(hive_data_t));

    // إرسال للطابور من الـ ISR
    if (esp_now_queue) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(esp_now_queue, &received, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// مهمة معالجة الطابور
static void esp_now_processor_task(void *pv) {
    hive_data_t data;
    while (1) {
        if (xQueueReceive(esp_now_queue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (data_callback) {
                data_callback(&data);
            }
        }
    }
}

void espnow_handler_init(void) {
    // إنشاء الطابور
    esp_now_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(hive_data_t));
    if (!esp_now_queue) {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (esp_now_init() != ESP_OK) {
        ESP_LOGE(TAG, "ESP-NOW init failed");
        return;
    }

    esp_now_register_recv_cb(on_data_receive);
    
    // إنشاء مهمة معالجة الطابور على Core 1
    xTaskCreatePinnedToCore(esp_now_processor_task, "esp_now_proc", 4096, NULL, 3, NULL, 1);
    
    ESP_LOGI(TAG, "ESP-NOW initialized with queue (size: %d)", ESPNOW_QUEUE_SIZE);
}

void espnow_handler_register_callback(espnow_data_callback_t cb) {
    data_callback = cb;
}

void espnow_handler_send_data(const hive_data_t* data, const uint8_t* dest_mac) {
    esp_now_send(dest_mac, (const uint8_t*)data, sizeof(hive_data_t));
}
// إعدادات التشفير لـ ESP-NOW
static uint8_t pmk[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
static uint8_t lmk[] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};

void espnow_enable_encryption(void) {
    esp_now_set_pmk(pmk);
    ESP_LOGI(TAG, "🔐 ESP-NOW encryption enabled");
}
