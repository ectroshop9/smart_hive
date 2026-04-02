#include "espnow_handler.h"
#include "esp_log.h"

static const char *TAG = "ESPNOW";

void espnow_handler_init(void)
{
    ESP_LOGI(TAG, "ESP-NOW handler initialized");
}

void espnow_handler_register_callback(void (*cb)(hive_data_t *data))
{
    ESP_LOGI(TAG, "Callback registered");
}
