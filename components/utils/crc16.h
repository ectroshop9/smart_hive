#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// CRC16-IBM (Polynomial 0x8005, Initial 0xFFFF)
static inline uint16_t crc16_compute(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;  // Polynomial reversed
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// لحساب CRC16 لـ SavedState (تتجاوز حقل checksum)
#define CRC16_COMPUTE_STATE(state) crc16_compute((const uint8_t*)&(state), offsetof(typeof(state), checksum))

#ifdef __cplusplus
}
#endif