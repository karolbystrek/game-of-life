#include "app.h"
#include "app_state.h"
#include "game.h"
#include "ui.h"
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void clean_res() {
  shm_unlink(SHM_NAME);
  sem_unlink(SEM_NAME);
}

AppState *app_init(const AppConfig *cfg) {
  AppState *app = calloc(1, sizeof(AppState));
  if (!app)
    return NULL;

  Game *tmp = cfg->filename ? load_game_from_file(cfg->filename)
                            : init_game(cfg->width, cfg->height);
  if (!tmp) {
    fprintf(stderr, "Init failed\n");
    exit(EXIT_FAILURE);
  }
  if (!cfg->filename && cfg->random_mode)
    randomize_game(tmp);

  int w = tmp->width, h = tmp->height;
  size_t g_sz = w * h * sizeof(bool);
  size_t shm_sz = sizeof(SharedState) + 3 * g_sz;

  clean_res();

  app->shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (app->shm_fd == -1)
    exit(EXIT_FAILURE);

  if (ftruncate(app->shm_fd, shm_sz) == -1)
    exit(EXIT_FAILURE);

  void *base =
      mmap(NULL, shm_sz, PROT_READ | PROT_WRITE, MAP_SHARED, app->shm_fd, 0);
  if (base == MAP_FAILED)
    exit(EXIT_FAILURE);

  app->shared = (SharedState *)base;
  app->grid_buffers[0] = (bool *)((char *)base + sizeof(SharedState));
  app->grid_buffers[1] = (bool *)((char *)base + sizeof(SharedState) + g_sz);
  app->initial_buffer = (bool *)((char *)base + sizeof(SharedState) + 2 * g_sz);

  app->shared->width = w;
  app->shared->height = h;
  app->shared->is_running = true;
  app->shared->is_paused = true;
  app->shared->simulation_speed_ns = 100000000L;
  app->shared->epoch = 0;
  app->shared->current_buffer_index = 0;

  memcpy(app->grid_buffers[0], tmp->grid, g_sz);
  memcpy(app->grid_buffers[1], tmp->grid, g_sz);
  memcpy(app->initial_buffer, tmp->grid, g_sz);

  free_game(tmp);

  app->sem_mutex = sem_open(SEM_NAME, O_CREAT, 0666, 1);
  if (app->sem_mutex == SEM_FAILED)
    exit(EXIT_FAILURE);

  app->cursor.x = w / 2;
  app->cursor.y = h / 2;
  return app;
}

static void run_eng(AppState *app) {
  Game g;
  g.width = app->shared->width;
  g.height = app->shared->height;
  g.initial_grid = NULL;

  while (app->shared->is_running) {
    if (!app->shared->is_paused) {
      sem_wait(app->sem_mutex);
      int cur = app->shared->current_buffer_index;
      g.grid = app->grid_buffers[cur];
      g.next_grid = app->grid_buffers[!cur];

      step_game(&g);

      app->shared->current_buffer_index = !cur;
      app->shared->epoch++;
      sem_post(app->sem_mutex);
    }
    struct timespec ts = {0, app->shared->simulation_speed_ns};
    nanosleep(&ts, NULL);
  }
}

static void run_ui(AppState *app) {
  ui_init();
  while (app->shared->is_running) {
    ui_draw(app);
    ui_handle_input(app);
    struct timespec ts = {0, 10000000L};
    nanosleep(&ts, NULL);
  }
  ui_cleanup();
}

void app_run(AppState *app) {
  pid_t pid = fork();
  if (pid < 0)
    exit(EXIT_FAILURE);

  if (pid == 0) {
    run_eng(app);
    exit(0);
  } else {
    app->engine_pid = pid;
    run_ui(app);
    waitpid(pid, NULL, 0);
  }
}

void app_cleanup(AppState *app) {
  if (!app)
    return;
  if (app->engine_pid > 0) {
    kill(app->engine_pid, SIGTERM);
    waitpid(app->engine_pid, NULL, 0);
  }
  if (app->shared) {
    size_t sz = sizeof(SharedState) +
                3 * app->shared->width * app->shared->height * sizeof(bool);
    munmap(app->shared, sz);
  }
  if (app->shm_fd != -1)
    close(app->shm_fd);
  if (app->sem_mutex && app->sem_mutex != SEM_FAILED)
    sem_close(app->sem_mutex);
  clean_res();
  free(app);
}
