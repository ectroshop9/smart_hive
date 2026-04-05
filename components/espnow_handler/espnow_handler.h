#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../utils/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*espnow_data_callback_t)(const hive_data_t* data);

void espnow_handler_init(void);
void espnow_handler_register_callback(espnow_data_callback_t cb);
void espnow_handler_send_data(const hive_data_t* data, const uint8_t* dest_mac);

#ifdef __cplusplus
}
#endif

#endif
