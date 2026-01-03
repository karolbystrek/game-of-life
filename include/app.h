#ifndef APP_H
#define APP_H

#include "app_state.h"
#include "config.h"

AppState *app_init(const AppConfig *config);

void app_run(AppState *app);

void app_cleanup(AppState *app);

#endif
