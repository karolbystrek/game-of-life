#include "ui.h"
#include <game.h>
#include <ncurses.h>
#include <time.h>

#define MIN_DELAY 10000000L  // 10ms
#define MAX_DELAY 990000000L // 990ms
#define DELAY_STEP 10000000L // 10ms

#define PAIR_ALIVE 1
#define PAIR_DEAD 2
#define PAIR_UI 3

void ui_init(void) {
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

void ui_cleanup(void) { endwin(); }

void ui_draw(const AppState *app) {
  clear();

  Game *game = app->game;

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      bool is_cursor = (x == app->cursor.x && y == app->cursor.y);
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
  int delay_ms = app->simulation_speed_ns / 1000000;

  attron(COLOR_PAIR(PAIR_UI));
  mvhline(footer_y, 0, ACS_HLINE, game->width > 60 ? game->width : 60);

  attron(A_BOLD);
  mvprintw(footer_y + 1, 0, " STATUS   ");
  attroff(A_BOLD);
  printw("| Epoch: %-5d | State: %-7s | Speed: %3dms", app->epoch,
         app->is_paused ? "PAUSED" : "RUNNING", delay_ms);

  attron(A_BOLD);
  mvprintw(footer_y + 2, 0, " CONTROLS ");
  attroff(A_BOLD);
  printw("| [ARROWS] Move   [ENTER] Toggle   [SPACE] Step");

  mvprintw(footer_y + 3, 10, "| [P] Play/Pause  [S] Save State   [+/-] Speed");
  mvprintw(footer_y + 4, 10, "| [R] Reset  [Q] Quit");

  if (app->status_msg[0] != '\0') {
    attron(A_BOLD | A_REVERSE);
    mvprintw(footer_y + 6, 0, " %s ", app->status_msg);
    attroff(A_BOLD | A_REVERSE);
  }

  attroff(COLOR_PAIR(PAIR_UI));

  refresh();
}

void ui_handle_input(AppState *app) {
  int ch = getch();

  if (ch == ERR) {
    return;
  }

  switch (ch) {
  case 'q':
  case 'Q':
    app->is_running = false;
    break;

  case ' ':
    if (app->is_paused) {
      step_game(app->game);
      app->epoch++;
    }
    break;

  case 'p':
  case 'P':
    app->is_paused = !app->is_paused;
    break;

  case '+':
    if (app->simulation_speed_ns > MIN_DELAY) {
      app->simulation_speed_ns -= DELAY_STEP;
    }
    break;

  case '-':
    if (app->simulation_speed_ns < MAX_DELAY) {
      app->simulation_speed_ns += DELAY_STEP;
    }
    break;

  case 'r':
  case 'R':
    reset_game(app->game);
    app->epoch = 0;
    break;

  case 's':
  case 'S':
    save_game_state(app->game, app->status_msg);
    break;

  case KEY_UP:
    app->cursor.y = (app->cursor.y - 1 + app->game->height) % app->game->height;
    break;

  case KEY_DOWN:
    app->cursor.y = (app->cursor.y + 1) % app->game->height;
    break;

  case KEY_LEFT:
    app->cursor.x = (app->cursor.x - 1 + app->game->width) % app->game->width;
    break;

  case KEY_RIGHT:
    app->cursor.x = (app->cursor.x + 1) % app->game->width;
    break;

  case '\n':
  case '\r':
  case KEY_ENTER:
    toggle_cell(app->game, app->cursor.x, app->cursor.y);
    break;
  }
}
