#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef struct {
  int width;
  int height;
  bool *initial_grid;
  bool *grid;
  bool *next_grid;
} Game;

Game *load_game_from_file(const char *filename);

Game *init_game(int width, int height);

void reset_game(Game *game);

void randomize_game(Game *game);

void save_game_state(Game *game, char *status_buffer);

void toggle_cell(Game *game, int x, int y);

void step_game(Game *game);

void free_game(Game *game);

#endif
