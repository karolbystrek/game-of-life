#ifndef APP_STATE_H
#define APP_STATE_H

#include "game.h"
#include <stdbool.h>

typedef struct {
  int x;
  int y;
} Cursor;

typedef struct {
  Game *game;
  bool is_running;
  bool is_paused;
  long simulation_speed_ns;
  int epoch;
  char status_msg[256];
  Cursor cursor;
} AppState;

#endif
