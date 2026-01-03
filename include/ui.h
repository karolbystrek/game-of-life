#ifndef UI_H
#define UI_H

#include "app_state.h"

void ui_init(void);

void ui_cleanup(void);

void ui_draw(const AppState *app);

void ui_handle_input(AppState *app);

#endif
