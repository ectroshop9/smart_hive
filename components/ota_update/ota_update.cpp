#include "ota_update.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include <string.h>

static const char* TAG = "OTA";
static const esp_partition_t* update_partition = NULL;
static esp_ota_handle_t update_handle = 0;
static bool update_in_progress = false;

bool ota_update_start(void) {
    if (update_in_progress) {
        ESP_LOGW(TAG, "OTA update already in progress");
        return false;
    }
    
    update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        ESP_LOGE(TAG, "Failed to get update partition");
        return false;
    }
    
    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to begin OTA: %s", esp_err_to_name(err));
        return false;
    }
    
    update_in_progress = true;
    ESP_LOGI(TAG, "OTA update started");
    return true;
}

bool ota_update_write_chunk(const uint8_t* data, size_t len) {
    if (!update_in_progress) return false;
    
    esp_err_t err = esp_ota_write(update_handle, data, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write OTA chunk: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool ota_update_finish(void) {
    if (!update_in_progress) return false;
    
    esp_err_t err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to end OTA: %s", esp_err_to_name(err));
        return false;
    }
    
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
        return false;
    }
    
    update_in_progress = false;
    ESP_LOGI(TAG, "OTA update successful, restarting...");
    esp_restart();
    return true;
}

void ota_update_abort(void) {
    if (update_in_progress) {
        esp_ota_abort(update_handle);
        update_in_progress = false;
        ESP_LOGW(TAG, "OTA update aborted");
    }
}

bool ota_update_is_running(void) {
    return update_in_progress;
}
