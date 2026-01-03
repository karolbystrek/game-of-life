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
#define PAIR_DEAD 2
#define PAIR_UI 3

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
    init_pair(PAIR_UI, COLOR_CYAN, -1);
  }
}

void draw_game(Game *game, int epoch, bool is_paused, long simulation_speed_ns,
               const char *status_msg, int cursor_x, int cursor_y) {
  clear();

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      bool is_cursor = (x == cursor_x && y == cursor_y);
      if (is_cursor) {
        attron(A_REVERSE);
      }

      if (game->grid[y * game->width + x]) {
        attron(COLOR_PAIR(PAIR_ALIVE) | A_BOLD);
        mvaddch(y, x, '#');
        attroff(COLOR_PAIR(PAIR_ALIVE) | A_BOLD);
      } else {
        attron(COLOR_PAIR(PAIR_DEAD) | A_DIM);
        mvaddch(y, x, '.');
        attroff(COLOR_PAIR(PAIR_DEAD) | A_DIM);
      }

      if (is_cursor) {
        attroff(A_REVERSE);
      }
    }
  }

  int footer_y = game->height + 1;
  int delay_ms = simulation_speed_ns / 1000000;

  attron(COLOR_PAIR(PAIR_UI));
  mvhline(footer_y, 0, ACS_HLINE, game->width > 60 ? game->width : 60);

  attron(A_BOLD);
  mvprintw(footer_y + 1, 0, " STATUS   ");
  attroff(A_BOLD);
  printw("| Epoch: %-5d | State: %-7s | Speed: %3dms", epoch,
         is_paused ? "PAUSED" : "RUNNING", delay_ms);

  attron(A_BOLD);
  mvprintw(footer_y + 2, 0, " CONTROLS ");
  attroff(A_BOLD);
  printw("| [ARROWS] Move   [ENTER] Toggle   [SPACE] Step");

  mvprintw(footer_y + 3, 10, "| [P] Play/Pause  [S] Save State   [+/-] Speed");
  mvprintw(footer_y + 4, 10, "| [Q] Quit");

  if (status_msg && status_msg[0] != '\0') {
    attron(A_BOLD | A_REVERSE);
    mvprintw(footer_y + 6, 0, " %s ", status_msg);
    attroff(A_BOLD | A_REVERSE);
  }

  attroff(COLOR_PAIR(PAIR_UI));

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
  char status_msg[256] = {0};
  int cursor_x = game->width / 2;
  int cursor_y = game->height / 2;

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
      } else if (ch == 's' || ch == 'S') {
        save_game_state(game, status_msg);
      } else if (ch == KEY_UP) {
        cursor_y = (cursor_y - 1 + game->height) % game->height;
      } else if (ch == KEY_DOWN) {
        cursor_y = (cursor_y + 1) % game->height;
      } else if (ch == KEY_LEFT) {
        cursor_x = (cursor_x - 1 + game->width) % game->width;
      } else if (ch == KEY_RIGHT) {
        cursor_x = (cursor_x + 1) % game->width;
      } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
        toggle_cell(game, cursor_x, cursor_y);
      }
    }

    if (!is_paused) {
      step_game(game);
      epoch++;
    }

    draw_game(game, epoch, is_paused, simulation_speed_ns, status_msg, cursor_x,
              cursor_y);

    struct timespec ts = {0, simulation_speed_ns};
    nanosleep(&ts, NULL);
  }

  endwin();
  free_game(game);
  return 0;
}
