#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <optional>
#include <variant>
#include <time.h>
#include "crc16.h"

// ==================== الثوابت ====================
#define MAX_HIVES        200
#define MAX_HIVE_ID      (MAX_HIVES + 10)
#define MAX_OPTIONS_LEN  (MAX_HIVES * 16)
#define HIVE_NAME_LEN    20

// ==================== Pins ====================
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

// ==================== عتبات الإنذار ====================
#define TEMP_CRITICAL     40.0f
#define TEMP_WARNING      35.0f
#define HUM_CRITICAL      20.0f
#define HUM_WARNING       30.0f
#define GAS_CRITICAL      50
#define GAS_WARNING       30
#define BATTERY_CRITICAL  10
#define BATTERY_WARNING   20

// ==================== AI Settings ====================
#define AI_TASK_STACK_SIZE      8192
#define AI_TASK_PRIORITY        4
#define AI_TASK_WATCHDOG_TIMEOUT_MS 5000

// ==================== ESP-NOW ====================
#define WIFI_CHANNEL            1
#define ESPNOW_QUEUE_SIZE       50

// ==================== NVS ====================
#define NVS_MAGIC_NUMBER        0x48495645UL
#define NVS_VERSION             1UL

// ==================== Fixed-Point ====================
#define FIXED_POINT_SCALE       256.0f
#define FLOAT_TO_FIXED(f)       ((int32_t)((f) * FIXED_POINT_SCALE))
#define FIXED_TO_FLOAT(f)       (((float)(f)) / FIXED_POINT_SCALE)

// ==================== Joystick ====================
#define JOYSTICK_CENTER         2048
#define JOYSTICK_THRESHOLD      1500
#define JOYSTICK_COOLDOWN_MS    200

// ==================== هيكل بيانات الخلية (13 حساساً) ====================
typedef struct __attribute__((packed)) {
    int16_t  id;
    int32_t  weight_fixed;
    int32_t  temp_1_fixed;
    int32_t  temp_2_fixed;
    int32_t  temp_3_fixed;
    int32_t  hum_fixed;
    uint8_t  battery;
    uint8_t  gas;
    uint8_t  uv;
    bool     motion_entrance;
    bool     motion_inside;
    uint8_t  sound;
    uint8_t  vibration;
} hive_data_t;

// ==================== NVS Structures ====================
typedef struct __attribute__((packed)) {
    uint32_t magic_number;
    uint32_t version;
    uint32_t hive_count;
    uint32_t timestamp;
    uint32_t checksum;
    uint8_t  reserved[32];
} nvs_header_t;

typedef struct __attribute__((packed)) {
    uint16_t selected_hive;
    uint16_t wifi_channel;
    uint8_t  alarm_enabled;
    uint8_t  ai_enabled;
    uint8_t  brightness;
    uint8_t  reserved[4];
} system_config_t;

// ==================== أنواع الحالات (Strong Typing) ====================
enum class AlarmLevel : uint8_t {
    Safe = 0,
    Warning = 1,
    Critical = 2
};

enum class AIResult : uint8_t {
    Healthy = 0,
    QueenLost = 1,
    Varroa = 2,
    Anomaly = 3,
    Swarming = 4
};

// ==================== سياق الخلية ====================
struct HiveContext {
    float temperature_avg;
    float humidity;
    float weight;
    float weight_trend;
    uint8_t gas;
    uint8_t sound;
    uint8_t vibration;
    bool motion_entrance;
    bool motion_inside;
    uint8_t battery;
};

// ==================== حالات الخلية ====================
struct NormalState {
    float health_score;
    time_t last_check;
};

struct SwarmingState {
    float probability;
    time_t estimated_start;
    int consecutive_detections;
};

struct SickState {
    enum Type : uint8_t { Varroa = 0, Foulbrood = 1, Chalkbrood = 2, Unknown = 3 };
    Type type;
    float severity;
    float confidence;
};

struct QueenLostState {
    time_t estimated_since;
    float confidence;
};

struct CriticalState {
    enum Type : uint8_t { Temperature = 0, Gas = 1, Battery = 2, Motion = 3 };
    Type type;
    float value;
    float threshold;
};

using HiveState = std::variant<
    NormalState,
    SwarmingState,
    SickState,
    QueenLostState,
    CriticalState
>;

// ==================== حالة محفوظة في NVS (مع CRC16) ====================
struct __attribute__((packed)) SavedState {
    uint32_t magic;        // 0x53415645
    uint32_t timestamp;
    uint32_t hive_id;
    uint8_t  state_type;   // 0=Normal,1=Swarming,2=Sick,3=QueenLost,4=Critical
    uint8_t  critical_type;
    float    value;
    uint16_t checksum;
    
    static constexpr uint32_t MAGIC = 0x53415645;
    
    bool isValid() const {
        if (magic != MAGIC) return false;
        uint16_t computed = crc16_compute((const uint8_t*)this, offsetof(SavedState, checksum));
        return computed == checksum;
    }
    
    void updateChecksum() {
        checksum = crc16_compute((const uint8_t*)this, offsetof(SavedState, checksum));
    }
};

// ==================== UI Update Message ====================
typedef struct {
    hive_data_t data;
    int idx;
} ui_update_msg_t;

// ==================== Fixed-Point Accessors ====================
static inline float hive_get_weight(const hive_data_t* hive) { return FIXED_TO_FLOAT(hive->weight_fixed); }
static inline float hive_get_temp_1(const hive_data_t* hive) { return FIXED_TO_FLOAT(hive->temp_1_fixed); }
static inline float hive_get_temp_2(const hive_data_t* hive) { return FIXED_TO_FLOAT(hive->temp_2_fixed); }
static inline float hive_get_temp_3(const hive_data_t* hive) { return FIXED_TO_FLOAT(hive->temp_3_fixed); }
static inline float hive_get_hum(const hive_data_t* hive)    { return FIXED_TO_FLOAT(hive->hum_fixed); }

static inline void hive_set_weight(hive_data_t* hive, float v) { hive->weight_fixed = FLOAT_TO_FIXED(v); }
static inline void hive_set_temp_1(hive_data_t* hive, float v) { hive->temp_1_fixed = FLOAT_TO_FIXED(v); }
static inline void hive_set_temp_2(hive_data_t* hive, float v) { hive->temp_2_fixed = FLOAT_TO_FIXED(v); }
static inline void hive_set_temp_3(hive_data_t* hive, float v) { hive->temp_3_fixed = FLOAT_TO_FIXED(v); }
static inline void hive_set_hum(hive_data_t* hive, float v)    { hive->hum_fixed = FLOAT_TO_FIXED(v); }

// ==================== دوال مساعدة للحالات ====================
static inline const char* stateToString(const HiveState& state) {
    if (std::holds_alternative<NormalState>(state)) return "NORMAL";
    if (std::holds_alternative<SwarmingState>(state)) return "SWARMING";
    if (std::holds_alternative<SickState>(state)) return "SICK";
    if (std::holds_alternative<QueenLostState>(state)) return "QUEEN_LOST";
    if (std::holds_alternative<CriticalState>(state)) return "CRITICAL";
    return "UNKNOWN";
}

// تأكيد حجم الـ Struct في وقت الترجمة
static_assert(sizeof(SavedState) == 20, "SavedState size must be 20 bytes");