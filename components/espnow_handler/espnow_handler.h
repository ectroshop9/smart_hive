#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int id;
    float weight;
    float temp;
    float hum;
    int battery;
    int gas;
    int uv;
    bool motion;
    float dsTemp[3];
} hive_data_t;

void espnow_handler_init(void);
void espnow_handler_register_callback(void (*cb)(hive_data_t *data));

#endif
