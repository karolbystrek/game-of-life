#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include "game.h"

void setup_ncurses() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
}

void draw_game(Game *game, int generation, bool is_paused) {
    clear();
    
    for (int y = 0; y < game->height; y++) {
        for (int x = 0; x < game->width; x++) {
            if (game->grid[y * game->width + x]) {
                mvaddch(y, x, '#');
            } else {
                mvaddch(y, x, '.');
            }
        }
    }

    int footer_y = game->height + 1;
    if (is_paused) {
        mvprintw(footer_y, 0, "Epoch: %d | Controls: [P] Play, [Q] Quit, [Space] Step", generation);
    } else {
        mvprintw(footer_y, 0, "Epoch: %d | Controls: [P] Pause, [Q] Quit", generation);
    }

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
            }
        }

        if (!is_paused) {
            step_game(game);
            epoch++;
        }

        draw_game(game, epoch, is_paused);
        
        struct timespec ts = {0, 100000000L};
        nanosleep(&ts, NULL);
    }

    endwin();
    free_game(game);
    return 0;
}
