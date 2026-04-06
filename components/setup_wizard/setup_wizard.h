#ifndef SETUP_WIZARD_H
#define SETUP_WIZARD_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void setup_wizard_start(void);
bool setup_wizard_is_required(void);
void setup_wizard_handle_joystick(int delta);
bool setup_wizard_is_active(void);

#ifdef __cplusplus
}
#endif

#endif
