#include "app.h"
#include "game.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

AppState *app_init(const AppConfig *config) {
  AppState *app = calloc(1, sizeof(AppState));
  if (!app) {
    return NULL;
  }

  if (config->filename) {
    app->game = load_game_from_file(config->filename);
    if (!app->game) {
      printf("Error: Could not load game from file '%s'\n", config->filename);
      free(app);
      return NULL;
    }
  } else {
    app->game = init_game(config->width, config->height);
    if (!app->game) {
      printf("Error: Failed to initialize game\n");
      free(app);
      return NULL;
    }

    if (config->random_mode) {
      randomize_game(app->game);
    }
  }

  app->is_running = true;
  app->is_paused = true;
  app->simulation_speed_ns = 100000000L;
  app->epoch = 0;
  app->cursor.x = app->game->width / 2;
  app->cursor.y = app->game->height / 2;

  return app;
}

void app_run(AppState *app) {
  ui_init();

  while (app->is_running) {
    ui_handle_input(app);

    if (!app->is_paused) {
      step_game(app->game);
      app->epoch++;
    }

    ui_draw(app);

    struct timespec ts = {0, app->simulation_speed_ns};
    nanosleep(&ts, NULL);
  }

  ui_cleanup();
}

void app_cleanup(AppState *app) {
  if (app) {
    free_game(app->game);
    free(app);
  }
}
