#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <sys/types.h>
#include <semaphore.h>

#define SHM_NAME "/gol_shm"
#define SEM_NAME "/gol_sem"

typedef struct {
  int x;
  int y;
} Cursor;

typedef struct {
  bool is_running;
  bool is_paused;
  long simulation_speed_ns;
  int epoch;
  int width;
  int height;
  int current_buffer_index; 
} SharedState;

typedef struct {
  pid_t engine_pid;
  int shm_fd;
  SharedState *shared;
  bool *grid_buffers[2];
  bool *initial_buffer;
  sem_t *sem_mutex;
  char status_msg[256];
  Cursor cursor;
} AppState;

#endif
