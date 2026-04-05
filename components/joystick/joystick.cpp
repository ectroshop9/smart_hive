#include "joystick.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "../utils/constants.h"

static const char* TAG = "JOYSTICK";
static adc_oneshot_unit_handle_t adc_handle = nullptr;
static bool last_sel_state = true;
static TickType_t last_joystick_move = 0;
static TickType_t last_sel_press = 0;
static void (*selection_callback)(int) = nullptr;

void joystick_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << JOY_SEL_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, JOY_X_ADC, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, JOY_Y_ADC, &config));

    ESP_LOGI(TAG, "Joystick initialized (no calibration)");
}

int joystick_read_x(void) {
    int raw = 0;
    if (adc_oneshot_read(adc_handle, JOY_X_ADC, &raw) != ESP_OK) return JOYSTICK_CENTER;
    return raw;
}

int joystick_read_y(void) {
    int raw = 0;
    if (adc_oneshot_read(adc_handle, JOY_Y_ADC, &raw) != ESP_OK) return JOYSTICK_CENTER;
    return raw;
}

bool joystick_is_pressed(void) {
    return (gpio_get_level((gpio_num_t)JOY_SEL_GPIO) == 0);
}

void joystick_register_callback(void (*cb)(int)) {
    selection_callback = cb;
}

static void joystick_task(void *pv) {
    while (1) {
        int current_x = joystick_read_x();
        bool current_sel = joystick_is_pressed();
        TickType_t now = xTaskGetTickCount();

        if ((now - last_joystick_move) > pdMS_TO_TICKS(JOYSTICK_COOLDOWN_MS)) {
            bool moved = false;
            if (current_x > JOYSTICK_CENTER + JOYSTICK_THRESHOLD) {
                moved = true;
                if (selection_callback) selection_callback(1);
            } else if (current_x < JOYSTICK_CENTER - JOYSTICK_THRESHOLD) {
                moved = true;
                if (selection_callback) selection_callback(-1);
            }
            if (moved) last_joystick_move = now;
        }

        if (current_sel != last_sel_state) {
            if (current_sel && (now - last_sel_press) > pdMS_TO_TICKS(200)) {
                ESP_LOGI(TAG, "Joystick pressed");
                last_sel_press = now;
            }
            last_sel_state = current_sel;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void joystick_start_task(void) {
    xTaskCreatePinnedToCore(joystick_task, "joystick_task", 3072, nullptr, 4, nullptr, 1);
}
