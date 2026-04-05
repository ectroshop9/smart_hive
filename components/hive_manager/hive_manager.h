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
