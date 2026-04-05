#include "rw_lock.h"

void rw_lock_init(rw_lock_t* lock) {
    lock->mutex = xSemaphoreCreateMutex();
    lock->read_mutex = xSemaphoreCreateMutex();
    lock->readers = 0;
}

void rw_lock_read_acquire(rw_lock_t* lock) {
    xSemaphoreTake(lock->read_mutex, portMAX_DELAY);
    lock->readers++;
    if (lock->readers == 1) {
        xSemaphoreTake(lock->mutex, portMAX_DELAY);
    }
    xSemaphoreGive(lock->read_mutex);
}

void rw_lock_read_release(rw_lock_t* lock) {
    xSemaphoreTake(lock->read_mutex, portMAX_DELAY);
    lock->readers--;
    if (lock->readers == 0) {
        xSemaphoreGive(lock->mutex);
    }
    xSemaphoreGive(lock->read_mutex);
}

void rw_lock_write_acquire(rw_lock_t* lock) {
    xSemaphoreTake(lock->mutex, portMAX_DELAY);
}

void rw_lock_write_release(rw_lock_t* lock) {
    xSemaphoreGive(lock->mutex);
}
