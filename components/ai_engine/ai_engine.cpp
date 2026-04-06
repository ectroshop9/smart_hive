#include "ai_engine.h"
#include "esp_log.h"
#include "../utils/constants.h"
#include <cstdio>
#include <cstring>

namespace AiEngine {

static const char* TAG = "AI_ENGINE";
static char last_result[128] = "AI: Healthy";
static HiveState current_state = NormalState{100.0f, 0};
static HiveState previous_state = NormalState{100.0f, 0};
static StateChangeCallback state_callback = nullptr;
static bool running = false;

void init(void) {
    ESP_LOGI(TAG, "AI Engine initialized");
}

void start(void) { running = true; ESP_LOGI(TAG, "AI Engine started"); }
void stop(void) { running = false; ESP_LOGI(TAG, "AI Engine stopped"); }
void restart(void) { stop(); init(); start(); }
bool isRunning(void) { return running; }
void setDataReady(void) {}
const char* getLastResult(void) { return last_result; }
HiveState getCurrentState(void) { return current_state; }
void registerStateCallback(StateChangeCallback callback) { state_callback = callback; }
void report_false_positive(int hive_id, AIResult incorrect_result) {}
void set_latitude(float lat) {}
bool configure_system(const char* brand_name, int wilaya_id, const char* admin_password) { return true; }
bool authenticate(const char* password) { return true; }
const WilayaInfo* get_current_wilaya(void) { return nullptr; }
const char* get_wilaya_name(int wilaya_id) { return "Algeria"; }
float get_wilaya_max_temp(int wilaya_id) { return 43.0f; }
bool factory_reset(const char* password) { return false; }
bool is_configured(void) { return true; }
void set_region(const char* region) {}

void analyze(const HiveContext& ctx, int hive_id) {
    if(!running) return;
    snprintf(last_result, sizeof(last_result), "Hive %d: Temp=%.1f°C", hive_id, ctx.temperature_avg);
    ESP_LOGI(TAG, "%s", last_result);
}

const char* get_expert_statistics(void) {
    static char stats[128];
    snprintf(stats, sizeof(stats), "AI Engine running");
    return stats;
}

} // namespace AiEngine
