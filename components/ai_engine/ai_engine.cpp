#include "ai_engine.h"
#include "esp_log.h"
#include "../utils/constants.h"
#include <cstring>

namespace AiEngine {

static const char* TAG = "AI_ENGINE";
static char last_result[64] = "AI: Healthy";
static HiveState current_state = NormalState{100.0f, 0};

void init(void) {
    ESP_LOGI(TAG, "AI Engine initialized");
}

void start(void) {
    ESP_LOGI(TAG, "AI Engine started");
}

void stop(void) {
    ESP_LOGI(TAG, "AI Engine stopped");
}

void restart(void) {
    ESP_LOGI(TAG, "AI Engine restarted");
}

bool isRunning(void) {
    return true;
}

void setDataReady(void) {
    // Simplified
}

const char* getLastResult(void) {
    return last_result;
}

HiveState getCurrentState(void) {
    return current_state;
}

void registerStateCallback(StateChangeCallback callback) {
    // Simplified
}

} // namespace AiEngine
