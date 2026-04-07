#ifndef NVS_LOGGER_H
#define NVS_LOGGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NVS_LOG_MAX_EVENTS 50

void nvs_log_init(void);
void nvs_log_event(const char* event);
void nvs_log_alarm(const char* alarm, int hive_id);
void nvs_log_state_change(const char* old_state, const char* new_state, int hive_id);
const char* nvs_log_get_last_events(void);

#ifdef __cplusplus
}
#endif

#endif
