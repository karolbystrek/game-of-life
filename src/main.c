#include "game.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MIN_DELAY 10000000L  // 10ms
#define MAX_DELAY 990000000L // 990ms
#define DELAY_STEP 10000000L // 10ms steps

#define PAIR_ALIVE 1
#define PAIR_DEAD  2
#define PAIR_UI    3

void setup_ncurses() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);

  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(PAIR_ALIVE, COLOR_MAGENTA, -1);
    init_pair(PAIR_DEAD, COLOR_WHITE, -1);
    init_pair(PAIR_UI, COLOR_BLACK, -1);
  }
}

void draw_game(Game *game, int generation, bool is_paused,
               long simulation_speed_ns) {
  clear();

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      if (game->grid[y * game->width + x]) {
        attron(COLOR_PAIR(PAIR_ALIVE) | A_BOLD);
        mvaddch(y, x, '#');
        attroff(COLOR_PAIR(PAIR_ALIVE) | A_BOLD);
      } else {
        attron(COLOR_PAIR(PAIR_DEAD) | A_DIM);
        mvaddch(y, x, '.');
        attroff(COLOR_PAIR(PAIR_DEAD) | A_DIM);
      }
    }
  }

  int footer_y = game->height + 1;
  int delay_ms = simulation_speed_ns / 1000000;

  attron(COLOR_PAIR(PAIR_UI) | A_BOLD);
  if (is_paused) {
    mvprintw(footer_y, 0,
             "Epoch: %d | Delay: %dms | Controls: [P] Play, [Q] Quit, "
             "[-/+] Speed, [Space] Step",
             generation, delay_ms);
  } else {
    mvprintw(footer_y, 0,
             "Epoch: %d | Delay: %dms | Controls: [P] Pause, [Q] Quit, "
             "[-/+] Speed",
             generation, delay_ms);
  }
  attroff(COLOR_PAIR(PAIR_UI) | A_BOLD);

  refresh();
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <input_file>\n", argv[0]);
    return 1;
  }

  Game *game = load_game_from_file(argv[1]);
  if (!game) {
    printf("Error: Could not load game from file '%s'\n", argv[1]);
    return 1;
  }

  setup_ncurses();

  bool is_running = true;
  bool is_paused = true;
  long simulation_speed_ns = 100000000L;
  int epoch = 0;

  while (is_running) {
    int ch = getch();

    if (ch != ERR) {
      if (ch == 'q' || ch == 'Q') {
        is_running = false;
      } else if (ch == ' ') {
        if (is_paused) {
          step_game(game);
          epoch++;
        }
      } else if (ch == 'p' || ch == 'P') {
        is_paused = !is_paused;
      } else if (ch == '+') {
        if (simulation_speed_ns > MIN_DELAY) {
          simulation_speed_ns -= DELAY_STEP;
        }
      } else if (ch == '-') {
        if (simulation_speed_ns < MAX_DELAY) {
          simulation_speed_ns += DELAY_STEP;
        }
      }
    }

    if (!is_paused) {
      step_game(game);
      epoch++;
    }

    draw_game(game, epoch, is_paused, simulation_speed_ns);

    struct timespec ts = {0, simulation_speed_ns};
    nanosleep(&ts, NULL);
  }

  endwin();
  free_game(game);
  return 0;
}
