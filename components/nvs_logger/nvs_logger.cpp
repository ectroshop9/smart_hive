#include "nvs_logger.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <ctime>

static const char* TAG = "NVS_LOG";
static char log_buffer[2048];
static char event_buffer[NVS_LOG_MAX_EVENTS][128];
static int event_index = 0;

void nvs_log_init(void) {
    memset(event_buffer, 0, sizeof(event_buffer));
    event_index = 0;
    ESP_LOGI(TAG, "NVS Logger initialized");
}

void nvs_log_event(const char* event) {
    time_t now;
    time(&now);
    struct tm* tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    
    snprintf(event_buffer[event_index % NVS_LOG_MAX_EVENTS], 128, "[%s] %s", timestamp, event);
    event_index++;
    
    ESP_LOGI(TAG, "📝 %s", event);
}

void nvs_log_alarm(const char* alarm, int hive_id) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "🚨 ALARM: %s (Hive %d)", alarm, hive_id);
    nvs_log_event(buffer);
}

void nvs_log_state_change(const char* old_state, const char* new_state, int hive_id) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "🔄 Hive %d: %s → %s", hive_id, old_state, new_state);
    nvs_log_event(buffer);
}

const char* nvs_log_get_last_events(void) {
    log_buffer[0] = '\0';
    int count = event_index > NVS_LOG_MAX_EVENTS ? NVS_LOG_MAX_EVENTS : event_index;
    int start = event_index - count;
    
    for (int i = 0; i < count; i++) {
        strcat(log_buffer, event_buffer[(start + i) % NVS_LOG_MAX_EVENTS]);
        strcat(log_buffer, "\n");
    }
    return log_buffer;
}
