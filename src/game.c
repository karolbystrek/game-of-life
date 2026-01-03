#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static bool *allocate_grid(int width, int height) {
  bool *grid = calloc(height * width, sizeof(bool));
  if (!grid) {
    return NULL;
  }

  return grid;
}

static void free_grid(bool *grid) {
  if (!grid) {
    return;
  }

  free(grid);
}

Game *init_game(int width, int height) {
  Game *game = malloc(sizeof(Game));
  if (!game) {
    return NULL;
  }

  game->width = width;
  game->height = height;
  game->grid = allocate_grid(width, height);
  game->next_grid = allocate_grid(width, height);

  if (!game->grid || !game->next_grid) {
    if (game->grid) {
      free_grid(game->grid);
    }
    if (game->next_grid) {
      free_grid(game->next_grid);
    }
    free(game);
    return NULL;
  }

  return game;
}

void free_game(Game *game) {
  if (!game) {
    return;
  }

  free_grid(game->grid);
  free_grid(game->next_grid);
  free(game);
}

Game *load_game_from_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f)
    return NULL;

  int max_width = 0;
  int height = 0;
  int current_width = 0;
  char ch;

  while ((ch = fgetc(f)) != EOF) {
    if (ch == '\n') {
      height++;
      if (current_width > max_width) {
        max_width = current_width;
      }
      current_width = 0;
    } else {
      current_width++;
    }
  }

  if (current_width > 0) {
    height++;
    if (current_width > max_width) {
      max_width = current_width;
    }
  }

  if (max_width == 0 || height == 0) {
    fclose(f);
    return NULL;
  }

  //reset file to the beginning
  rewind(f);

  Game *game = init_game(max_width, height);
  if (!game) {
    fclose(f);
    return NULL;
  }

  int y = 0;
  int x = 0;
  while ((ch = fgetc(f)) != EOF) {
    if (ch == '\n') {
      y++;
      x = 0;
    } else {
      if (x < max_width && y < height) {
        if (ch == '#') {
          game->grid[y * game->width + x] = true;
        } else {
          game->grid[y * game->width + x] = false;
        }
      }
      x++;
    }
  }

  fclose(f);
  return game;
}

void save_game_state(Game *game, char *status_buffer) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char filename[100];

  strftime(filename, sizeof(filename), "snapshot_%Y%m%d_%H%M%S.txt", t);

  FILE *f = fopen(filename, "w");
  if (!f) {
    snprintf(status_buffer, 256, "Error: Failed to save to %s", filename);
    return;
  }

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      if (game->grid[y * game->width + x]) {
        fputc('#', f);
      } else {
        fputc('.', f);
      }
    }
    fputc('\n', f);
  }

  fclose(f);
  snprintf(status_buffer, 256, "Success: Saved to %s", filename);
}

static int count_neighbors(Game *game, int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (j == 0 && i == 0) {
        continue;
      }

      //wrap around the edges
      int row = (y + i + game->height) % game->height;
      int col = (x + j + game->width) % game->width;

      if (game->grid[row * game->width + col]) {
        count++;
      }
    }
  }
  return count;
}

static void swap_grids(Game *game) {
  bool *temp = game->grid;
  game->grid = game->next_grid;
  game->next_grid = temp;
}

void step_game(Game *game) {
  for (int h = 0; h < game->height; h++) {
    for (int w = 0; w < game->width; w++) {
      int neighbors = count_neighbors(game, w, h);
      bool alive = game->grid[h * game->width + w];

      if (alive) {
        if (neighbors < 2 || neighbors > 3) {
          game->next_grid[h * game->width + w] = false;
        } else {
          game->next_grid[h * game->width + w] = true;
        }
      } else {
        if (neighbors == 3) {
          game->next_grid[h * game->width + w] = true;
        } else {
          game->next_grid[h * game->width + w] = false;
        }
      }
    }
  }

  swap_grids(game);
}
