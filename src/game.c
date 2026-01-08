#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static bool *allocate_grid(int width, int height) {
  return calloc(height * width, sizeof(bool));
}

Game *init_game(int width, int height) {
  Game *game = malloc(sizeof(Game));
  if (!game)
    return NULL;

  game->width = width;
  game->height = height;
  game->initial_grid = allocate_grid(width, height);
  game->grid = allocate_grid(width, height);
  game->next_grid = allocate_grid(width, height);

  if (!game->grid || !game->next_grid || !game->initial_grid) {
    free_game(game);
    return NULL;
  }

  return game;
}

void free_game(Game *game) {
  if (!game)
    return;
  free(game->grid);
  free(game->next_grid);
  free(game->initial_grid);
  free(game);
}

Game *load_game_from_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file)
    return NULL;

  int max_width = 0, max_height = 0, width = 0, height;
  while ((height = fgetc(file)) != EOF) {
    if (height == '\n') {
      max_height++;
      if (width > max_width)
        max_width = width;
      width = 0;
    } else {
      width++;
    }
  }
  if (width > 0) {
    max_height++;
    if (width > max_width)
      max_width = width;
  }

  if (max_width == 0 || max_height == 0) {
    fclose(file);
    return NULL;
  }

  rewind(file);
  Game *game = init_game(max_width, max_height);
  if (!game) {
    fclose(file);
    return NULL;
  }

  int x = 0, y = 0;
  while ((height = fgetc(file)) != EOF) {
    if (height == '\n') {
      y++;
      x = 0;
    } else {
      if (x < max_width && y < max_height) {
        bool alive = (height == '#');
        game->grid[y * game->width + x] = alive;
        game->initial_grid[y * game->width + x] = alive;
      }
      x++;
    }
  }

  fclose(file);
  return game;
}

void save_game_state(Game *game, char *status_buffer) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char filename[100];
  strftime(filename, sizeof(filename), "snapshot_%Y%m%d_%H%M%S.txt", t);

  FILE *file = fopen(filename, "w");
  if (!file) {
    if (status_buffer)
      snprintf(status_buffer, 256, "Error saving %s", filename);
    return;
  }

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      fputc(game->grid[y * game->width + x] ? '#' : '.', file);
    }
    fputc('\n', file);
  }

  fclose(file);
  if (status_buffer)
    snprintf(status_buffer, 256, "Saved %s", filename);
}

static int count_neighbors(Game *game, int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (i == 0 && j == 0)
        continue;
      int row = (y + i + game->height) % game->height;
      int col = (x + j + game->width) % game->width;
      if (game->grid[row * game->width + col])
        count++;
    }
  }
  return count;
}

void step_game(Game *game) {
  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      int n = count_neighbors(game, x, y);
      bool alive = game->grid[y * game->width + x];
      game->next_grid[y * game->width + x] =
          (alive && (n == 2 || n == 3)) || (!alive && n == 3);
    }
  }

  bool *tmp = game->grid;
  game->grid = game->next_grid;
  game->next_grid = tmp;
}

void toggle_cell(Game *game, int x, int y) {
  if (x >= 0 && x < game->width && y >= 0 && y < game->height) {
    game->grid[y * game->width + x] = !game->grid[y * game->width + x];
  }
}

void randomize_game(Game *game) {
  srand(time(NULL));
  for (int i = 0; i < game->width * game->height; i++) {
    game->grid[i] = (rand() % 5 == 0);
  }
}

void reset_game(Game *game) {
  memcpy(game->grid, game->initial_grid,
         game->width * game->height * sizeof(bool));
}
