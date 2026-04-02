#ifndef HIVE_MANAGER_H
#define HIVE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "espnow_handler.h"

#define MAX_HIVES 10

void hive_manager_init(void);
void hive_manager_update(hive_data_t *data);
int hive_manager_get_count(void);
hive_data_t* hive_manager_get_hive(int index);

#endif
