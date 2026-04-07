#ifndef HIVE_MANAGER_H
#define HIVE_MANAGER_H

#include "../utils/types.h"
#include <optional>

namespace HiveManager {

void init(void);
void saveAll(void);
void loadAll(void);
int getCount(void);
std::optional<hive_data_t> getHive(int index);
hive_data_t* getHiveMutable(int index);
void updateHive(const hive_data_t& data);
void addTestData(void);
void setSelected(int idx);
int getSelected(void);

} // namespace HiveManager

#endif

// دوال حماية البيانات (Mutex)
void hive_manager_lock_read(void);
void hive_manager_unlock_read(void);
void hive_manager_lock_write(void);
void hive_manager_unlock_write(void);
