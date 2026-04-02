#include "hive_manager.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "HIVE_MGR";

void hive_manager_init(void)
{
    ESP_LOGI(TAG, "Hive manager initialized");
}

void hive_manager_update(hive_data_t *data)
{
    ESP_LOGI(TAG, "Updating hive ID: %d", data->id);
}

int hive_manager_get_count(void)
{
    return 0;
}

hive_data_t* hive_manager_get_hive(int index)
{
    return NULL;
}
