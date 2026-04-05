#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void joystick_init(void);
int joystick_read_x(void);
int joystick_read_y(void);
bool joystick_is_pressed(void);
void joystick_start_task(void);
void joystick_register_callback(void (*cb)(int));

#ifdef __cplusplus
}
#endif

#endif
