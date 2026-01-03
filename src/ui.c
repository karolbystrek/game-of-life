#include "ui.h"
#include "app_state.h"
#include "game.h"
#include <ncurses.h>
#include <time.h>
#include <string.h>

#define MIN_DELAY 10000000L
#define MAX_DELAY 990000000L
#define DELAY_STEP 10000000L

#define P_ALIVE 1
#define P_DEAD 2
#define P_UI 3

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
    init_pair(P_ALIVE, COLOR_MAGENTA, -1);
    init_pair(P_DEAD, COLOR_WHITE, -1);
    init_pair(P_UI, COLOR_CYAN, -1);
  }
}

void ui_cleanup(void) { endwin(); }

static Game get_view(const AppState *app) {
    Game g = {
      .width = app->shared->width,
      .height = app->shared->height,
      .grid = app->grid_buffers[app->shared->current_buffer_index],
      .next_grid = app->grid_buffers[!app->shared->current_buffer_index],
      .initial_grid = app->initial_buffer
    };
    return g;
}

void ui_draw(const AppState *app) {
  clear();
  Game g = get_view(app);

  for (int y = 0; y < g.height; y++) {
    for (int x = 0; x < g.width; x++) {
      bool cursor = (x == app->cursor.x && y == app->cursor.y);
      if (cursor) attron(A_REVERSE);
      
      if (g.grid[y * g.width + x]) {
        attron(COLOR_PAIR(P_ALIVE) | A_BOLD);
        mvaddch(y, x, '#');
        attroff(COLOR_PAIR(P_ALIVE) | A_BOLD);
      } else {
        attron(COLOR_PAIR(P_DEAD) | A_DIM);
        mvaddch(y, x, '.');
        attroff(COLOR_PAIR(P_DEAD) | A_DIM);
      }
      
      if (cursor) attroff(A_REVERSE);
    }
  }

  int fy = g.height + 1;
  int ms = app->shared->simulation_speed_ns / 1000000;

  attron(COLOR_PAIR(P_UI));
  mvhline(fy, 0, ACS_HLINE, g.width > 60 ? g.width : 60);

  attron(A_BOLD);
  mvprintw(fy + 1, 0, " STATUS   ");
  attroff(A_BOLD);
  printw("| Eph: %-5d | St: %-7s | Spd: %3dms", app->shared->epoch,
         app->shared->is_paused ? "PAUSED" : "RUNNING", ms);

  attron(A_BOLD);
  mvprintw(fy + 2, 0, " CONTROLS ");
  attroff(A_BOLD);
  printw("| [ARROWS] Move [ENT] Toggle [SPC] Step");
  mvprintw(fy + 3, 10, "| [P] Play/Pause [S] Save [+/-] Spd");
  mvprintw(fy + 4, 10, "| [R] Reset [Q] Quit");

  if (app->status_msg[0]) {
    attron(A_BOLD | A_REVERSE);
    mvprintw(fy + 6, 0, " %s ", app->status_msg);
    attroff(A_BOLD | A_REVERSE);
  }
  attroff(COLOR_PAIR(P_UI));
  refresh();
}

void ui_handle_input(AppState *app) {
  int ch = getch();
  if (ch == ERR) return;
  
  Game g = get_view(app);
  sem_wait(app->sem_mutex);

  switch (ch) {
    case 'q': case 'Q': 
      app->shared->is_running = false; 
      break;
    case ' ':
      if (app->shared->is_paused) {
        step_game(&g);
        app->shared->current_buffer_index = !app->shared->current_buffer_index;
        app->shared->epoch++;
      }
      break;
    case 'p': case 'P': 
      app->shared->is_paused = !app->shared->is_paused; 
      break;
    case '+': 
      if (app->shared->simulation_speed_ns > MIN_DELAY) 
        app->shared->simulation_speed_ns -= DELAY_STEP; 
      break;
    case '-': 
      if (app->shared->simulation_speed_ns < MAX_DELAY) 
        app->shared->simulation_speed_ns += DELAY_STEP; 
      break;
    case 'r': case 'R':
      reset_game(&g);
      if (g.next_grid) memcpy(g.next_grid, g.grid, g.width * g.height * sizeof(bool));
      app->shared->epoch = 0;
      break;
    case 's': case 'S':
      save_game_state(&g, app->status_msg);
      break;
    case KEY_UP:    app->cursor.y = (app->cursor.y - 1 + g.height) % g.height; break;
    case KEY_DOWN:  app->cursor.y = (app->cursor.y + 1) % g.height; break;
    case KEY_LEFT:  app->cursor.x = (app->cursor.x - 1 + g.width) % g.width; break;
    case KEY_RIGHT: app->cursor.x = (app->cursor.x + 1) % g.width; break;
    case '\n': case '\r': case KEY_ENTER:
      toggle_cell(&g, app->cursor.x, app->cursor.y);
      break;
  }

  sem_post(app->sem_mutex);
}
