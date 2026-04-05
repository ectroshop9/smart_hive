#ifndef RW_LOCK_H
#define RW_LOCK_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t read_mutex;
    int readers;
} rw_lock_t;

void rw_lock_init(rw_lock_t* lock);
void rw_lock_read_acquire(rw_lock_t* lock);
void rw_lock_read_release(rw_lock_t* lock);
void rw_lock_write_acquire(rw_lock_t* lock);
void rw_lock_write_release(rw_lock_t* lock);

#ifdef __cplusplus
}
#endif

#endif
