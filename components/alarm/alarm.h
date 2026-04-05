#ifndef ALARM_H
#define ALARM_H

#include "../utils/types.h"

namespace Alarm {

void init(void);
void request(AlarmLevel level);
void stop(void);
bool isActive(void);
AlarmLevel getCurrentLevel(void);
void startTask(void);

} // namespace Alarm

#endif
