#include "joystick.h"
#include "esp_log.h"

static const char *TAG = "JOYSTICK";

void joystick_init(void)
{
    ESP_LOGI(TAG, "Joystick initialized");
}

int joystick_get_selection(void)
{
    return 0;
}
