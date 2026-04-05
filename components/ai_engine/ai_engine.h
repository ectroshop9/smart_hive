#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "../utils/types.h"
#include <functional>

namespace AiEngine {

using StateChangeCallback = std::function<void(const HiveState& oldState, const HiveState& newState)>;

void init(void);
void start(void);
void stop(void);
void restart(void);
bool isRunning(void);
void setDataReady(void);
const char* getLastResult(void);
HiveState getCurrentState(void);
void registerStateCallback(StateChangeCallback callback);

} // namespace AiEngine

#endif
