#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef struct {
    int width;
    int height;
    bool *grid;
    bool *next_grid;
} Game;

Game *load_game_from_file(const char *filename);

Game *init_game(int width, int height);

void save_game_state(Game *game, char *status_buffer);

void step_game(Game *game);

void free_game(Game *game);

#endif
