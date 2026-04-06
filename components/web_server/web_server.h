#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool start_web_server(void);
void stop_web_server(void);
void web_server_update_data(float temp, float hum, float weight, float batt, 
                            int sound, int gas, int uv, int vibration,
                            bool motion_entrance, bool motion_inside,
                            int colony_strength, int health_score,
                            float top_temp, float mid_temp, float bottom_temp);
const char* get_server_ip(void);

#ifdef __cplusplus
}
#endif

#endif
