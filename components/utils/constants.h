#pragma once

#define MAX_HIVES        200
#define MAX_HIVE_ID      (MAX_HIVES + 10)
#define MAX_OPTIONS_LEN  (MAX_HIVES * 16)
#define HIVE_NAME_LEN    20

#define PIN_NUM_MOSI     13
#define PIN_NUM_CLK      12
#define PIN_NUM_CS       10
#define PIN_NUM_DC       11
#define PIN_NUM_RST      15
#define LCD_H_RES        320
#define LCD_V_RES        240

#define BUZZER_GPIO      4
#define LED_ALERT_GPIO   5

#define JOY_SEL_GPIO       1
#define JOY_X_ADC          ADC_CHANNEL_2
#define JOY_Y_ADC          ADC_CHANNEL_3
#define JOYSTICK_DEADZONE  500
#define JOYSTICK_THRESHOLD 1500
#define JOYSTICK_CENTER    2048
#define JOYSTICK_COOLDOWN_MS 200

#define TEMP_CRITICAL     40.0f
#define TEMP_WARNING      35.0f
#define HUM_CRITICAL      20.0f
#define HUM_WARNING       30.0f
#define GAS_CRITICAL      50
#define GAS_WARNING       30
#define BATTERY_CRITICAL  10
#define BATTERY_WARNING   20

#define WIFI_CHANNEL 1

#define AI_FILTER_WINDOW             3
#define AI_CONSECUTIVE_THRESHOLD     2
#define AI_TASK_WATCHDOG_TIMEOUT_MS  5000
#define AI_TASK_STACK_SIZE           8192
#define AI_TASK_PRIORITY             3

#define DROPDOWN_UPDATE_INTERVAL_SEC 10

#define NVS_MAGIC_NUMBER  0x48495645UL
#define NVS_VERSION       1UL

#define FIXED_POINT_SCALE  256.0f
#define FLOAT_TO_FIXED(f)  ((int32_t)((f) * FIXED_POINT_SCALE))
#define FIXED_TO_FLOAT(f)  (((float)(f)) / FIXED_POINT_SCALE)
