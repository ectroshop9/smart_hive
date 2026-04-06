#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "../utils/types.h"
#include <functional>

namespace AiEngine {

using StateChangeCallback = std::function<void(const HiveState& oldState, const HiveState& newState)>;

// دوال التحكم الأساسية
void init(void);
void start(void);
void stop(void);
void restart(void);
bool isRunning(void);

// معالجة البيانات
void setDataReady(void);
void analyze(const HiveContext& ctx, int hive_id);
const char* getLastResult(void);
HiveState getCurrentState(void);

// Callbacks والإحصائيات
void registerStateCallback(StateChangeCallback callback);
const char* get_expert_statistics(void);

// التعلم والتكيف
void report_false_positive(int hive_id, AIResult incorrect_result);
void set_latitude(float lat);

// النظام السيادي الجزائري
bool configure_system(const char* brand_name, int wilaya_id, const char* admin_password);
bool authenticate(const char* password);
const WilayaInfo* get_current_wilaya(void);
const char* get_wilaya_name(int wilaya_id);
float get_wilaya_max_temp(int wilaya_id);
bool factory_reset(const char* password);
bool is_configured(void);
void set_region(const char* region);

} // namespace AiEngine

#endif

// دوال النظام السيادي (مطلوبة)
const WilayaInfo* get_current_wilaya(void);
