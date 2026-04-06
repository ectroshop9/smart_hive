#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool ota_update_start(void);
bool ota_update_write_chunk(const uint8_t* data, size_t len);
bool ota_update_finish(void);
void ota_update_abort(void);
bool ota_update_is_running(void);

#ifdef __cplusplus
}
#endif

#endif
