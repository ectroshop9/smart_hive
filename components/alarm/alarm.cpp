#include "alarm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "../utils/constants.h"
#include "../utils/types.h"
#include <atomic>

namespace Alarm {

static std::atomic<bool> alarm_request_flag(false);
static std::atomic<AlarmLevel> current_level{AlarmLevel::Safe};
static int alarm_cooldown_counter = 0;
#define ALARM_COOLDOWN_CYCLES 10

void init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUZZER_GPIO) | (1ULL << LED_ALERT_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)BUZZER_GPIO, 0);
    gpio_set_level((gpio_num_t)LED_ALERT_GPIO, 0);
}

void request(AlarmLevel level) {
    if (alarm_cooldown_counter > 0) return;
    if (level == AlarmLevel::Safe) return;
    
    alarm_request_flag.store(true);
    current_level.store(level);
    alarm_cooldown_counter = ALARM_COOLDOWN_CYCLES;
}

void stop(void) {
    alarm_request_flag.store(false);
    current_level.store(AlarmLevel::Safe);
    gpio_set_level((gpio_num_t)BUZZER_GPIO, 0);
    gpio_set_level((gpio_num_t)LED_ALERT_GPIO, 0);
    alarm_cooldown_counter = 0;
}

bool isActive(void) {
    return alarm_request_flag.load();
}

AlarmLevel getCurrentLevel(void) {
    return current_level.load();
}

static void alarm_task(void *pv) {
    bool current_critical = false;
    int blink_counter = 0;
    const int blink_period = 20;

    while (1) {
        if (alarm_cooldown_counter > 0) alarm_cooldown_counter--;

        bool should_alarm = alarm_request_flag.load();
        AlarmLevel level = current_level.load();

        if (should_alarm) {
            if (level == AlarmLevel::Critical) {
                gpio_set_level((gpio_num_t)BUZZER_GPIO, 1);
                gpio_set_level((gpio_num_t)LED_ALERT_GPIO, 1);
                current_critical = true;
            } else if (level == AlarmLevel::Warning) {
                blink_counter = (blink_counter + 1) % blink_period;
                bool on_state = (blink_counter < blink_period / 2);
                gpio_set_level((gpio_num_t)BUZZER_GPIO, on_state ? 1 : 0);
                gpio_set_level((gpio_num_t)LED_ALERT_GPIO, on_state ? 1 : 0);
                current_critical = false;
            }
        } else {
            if (current_critical || blink_counter != 0) {
                gpio_set_level((gpio_num_t)BUZZER_GPIO, 0);
                gpio_set_level((gpio_num_t)LED_ALERT_GPIO, 0);
                blink_counter = 0;
                current_critical = false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void startTask(void) {
    xTaskCreatePinnedToCore(alarm_task, "alarm_task", 2048, nullptr, 5, nullptr, 0);
}

} // namespace Alarm
