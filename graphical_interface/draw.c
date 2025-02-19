#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include "../constants.h"
#include "draw.h"
#include "../parsing/cell_indexing.h"
#include "../backend/compute_unit.h"
#include "../data_structures/set.h"

DisplayState *state = NULL;

void draw_cell(const Cell* cell, const short y, const short x, const short width, const int color_pair, const int reverse) {
    if (cell->state == ZERO_ERROR) {
        wattron(state->grid_win, COLOR_PAIR(3));
        if (reverse) wattron(state->grid_win, A_REVERSE);
        mvwprintw(state->grid_win, y, x, "%-*s", width - 1, "ERR");
        wattroff(state->grid_win, COLOR_PAIR(3));
        if (reverse) wattroff(state->grid_win, A_REVERSE);
        return;
    }
    if (reverse) wattron(state->grid_win, A_REVERSE);
    if (color_pair) wattron(state->grid_win, COLOR_PAIR(color_pair));
    int temp = cell->value;
    short len_val = 0;
    while (temp) {
        len_val++;
        temp /= 10;
    }
    if (cell->value == 0) len_val=1;
    len_val += (cell->value < 0);
    if (!reverse) {
        if (len_val > width-1) {
            mvwprintw(state->grid_win, y, x, "%d..", cell->value/(int)pow(10, len_val - width + 3));
        } else {
            mvwprintw(state->grid_win, y, x, "%-*d", width - 1, cell->value);
        }
    } else {
        mvwprintw(state->grid_win, y, x, "%-*d", width - 1, cell->value);
    }
    if (color_pair) wattroff(state->grid_win, COLOR_PAIR(color_pair));
    if (reverse) wattroff(state->grid_win, A_REVERSE);
}

void draw_grid() {
    werase(state->grid_win);
    for (short j = 0; j < state->viewport.visible_cols; j++) {
        const short actual_col = j + state->viewport.start_col;
        char col_label[MAX_COL_LABEL];
        col_index_to_label(actual_col, col_label);
        wattron(state->grid_win, COLOR_PAIR(2));
        mvwprintw(state->grid_win, 0, (j + 1) * state->viewport.cell_width, "%s", col_label);
        wattroff(state->grid_win, COLOR_PAIR(2));
    }
    for (short i = 0; i < state->viewport.visible_rows; i++) {
        const short actual_row = i + state->viewport.start_row;
        for (short j = 0; j < state->viewport.visible_cols; j++) {
            const short actual_col = j + state->viewport.start_col;
            get_cell_value(actual_row, actual_col);
        }
    }
    for (short i = 0; i < state->viewport.visible_rows; i++) {
        const short actual_row = i + state->viewport.start_row;
        wattron(state->grid_win, COLOR_PAIR(2));
        mvwprintw(state->grid_win, i + 1, 0, "%4d", actual_row + 1);
        wattroff(state->grid_win, COLOR_PAIR(2));
        for (short j = 0; j < state->viewport.visible_cols; j++) {
            const short actual_col = j + state->viewport.start_col;
            const short x = (j + 1) * state->viewport.cell_width;
            const short y = i + 1;
            const bool cell_in_expr = cellWithinExpression(&get_cell(state->curr_row, state->curr_col)->expression, actual_row, actual_col);
            int color_pair = 0;
            int reverse = 0;
            if (state->mode == INTERACTIVE_MODE) {
                if (actual_row == state->curr_row && actual_col == state->curr_col) {
                    reverse = 1;
                }
                if (cell_in_expr) {
                    color_pair = 4;
                }
                if (get_cell(actual_row, actual_col)->state != ZERO_ERROR && get_cell(actual_row, actual_col)->state != CLEAN) {
                    color_pair = 1;
                } else if (get_cell(actual_row, actual_col)->state == ZERO_ERROR) {
                    color_pair = 3;
                }
            }
            draw_cell(get_cell(actual_row, actual_col), y, x, state->viewport.cell_width, color_pair, reverse);
        }
        if (state->mode == INTERACTIVE_MODE) {
            if (state->curr_row < state->viewport.start_row || state->curr_row >= state->viewport.start_row + state->viewport.visible_rows ||
                state->curr_col < state->viewport.start_col || state->curr_col >= state->viewport.start_col + state->viewport.visible_cols) {
                continue;
            }
            int color_pair_ = 0;
            int reverse_ = 1;
            if (get_cell(state->curr_row, state->curr_col)->state != ZERO_ERROR && get_cell(state->curr_row, state->curr_col)->state != CLEAN) {
                color_pair_ = 1;
            } else if (get_cell(state->curr_row, state->curr_col)->state == ZERO_ERROR) {
                color_pair_ = 3;
            }
            draw_cell(get_cell(state->curr_row, state->curr_col), state->curr_row - state->viewport.start_row + 1, (state->curr_col - state->viewport.start_col + 1) * state->viewport.cell_width, state->viewport.cell_width, color_pair_, reverse_);
        }
    }

    char curr_col_label[MAX_COL_LABEL];
    col_index_to_label(state->curr_col, curr_col_label);
    wrefresh(state->grid_win);
}

void draw_state() {
    werase(state->status_win);
    char curr_col_label[MAX_COL_LABEL];
    col_index_to_label(state->curr_col, curr_col_label);

    wattron(state->status_win, COLOR_PAIR(2));
    mvwprintw(state->status_win, 0, 0, "Current Cell:     ");
    mvwprintw(state->status_win, 1, 0, "Cell Value:       ");
    mvwprintw(state->status_win, 2, 0, "Cell Expression:  ");
    wattroff(state->status_win, COLOR_PAIR(2));

    mvwprintw(state->status_win, 0, 18, "%s%d", curr_col_label, state->curr_row + 1);
    mvwprintw(state->status_win, 1, 18, "%d", get_raw_value(state->curr_row, state->curr_col));
    mvwprintw(state->status_win, 2, 18, "%s", get_expression_string(&get_cell(state->curr_row, state->curr_col)->expression));
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependencies:     ");
    wattroff(state->status_win, COLOR_PAIR(2));
    size_t dependency_count = get_cell(state->curr_row, state->curr_col)->dependency_count;
    int excess = 0;
    if (dependency_count > 20) {
        dependency_count = 20;
        excess = 1;
    }
    for (int i = 0; i < dependency_count; i++) {
        char col_label[MAX_COL_LABEL];
        col_index_to_label(get_cell(state->curr_row, state->curr_col)->dependencies[i]->col, col_label);
        int done = 0;
        if (get_cell(get_cell(state->curr_row, state->curr_col)->dependencies[i]->row, get_cell(state->curr_row, state->curr_col)->dependencies[i]->col)->state == ZERO_ERROR) {
            done = 1;
            wattron(state->status_win, COLOR_PAIR(3));
        } else if (get_cell(get_cell(state->curr_row, state->curr_col)->dependencies[i]->row, get_cell(state->curr_row, state->curr_col)->dependencies[i]->col)->state != CLEAN) {
            done = 2;
            wattron(state->status_win, COLOR_PAIR(1));
        }
        wprintw(state->status_win, "%s%d ", col_label, get_cell(state->curr_row, state->curr_col)->dependencies[i]->row + 1);
        if (done == 1) {
            wattroff(state->status_win, COLOR_PAIR(3));
        } else if (done == 2) {
            wattroff(state->status_win, COLOR_PAIR(1));
        }
    }
    if (excess) {
        wprintw(state->status_win, "...");
    }
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependency Count: ");
    wattroff(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "%lu", get_cell(state->curr_row, state->curr_col)->dependency_count);
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependants:       ");
    wattroff(state->status_win, COLOR_PAIR(2));
    SetIterator *iter = set_iterator_create(get_cell(state->curr_row, state->curr_col)->dependants);
    Cell *cell;
    while ((cell = set_iterator_next(iter)) != NULL) {
        char col_label[MAX_COL_LABEL];
        col_index_to_label(cell->col, col_label);
        int done = 0;
        if (cell->state == ZERO_ERROR) {
            done = 1;
            wattron(state->status_win, COLOR_PAIR(3));
        } else if (cell->state != CLEAN) {
            done = 2;
            wattron(state->status_win, COLOR_PAIR(1));
        }
        wprintw(state->status_win, "%s%d ", col_label, cell->row + 1);
        if (done == 1) {
            wattroff(state->status_win, COLOR_PAIR(3));
        } else if (done == 2) {
            wattroff(state->status_win, COLOR_PAIR(1));
        }
    }
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependant Count:  ");
    wattroff(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "%lu", set_size(get_cell(state->curr_row, state->curr_col)->dependants));
    set_iterator_destroy(iter);
    wrefresh(state->status_win);
}

void draw_history() {
    wclear(state->command_win);
    const int start_row = CMD_HISTORY_SIZE - state->cmd_history_count;
    for (int i = 0; i < state->cmd_history_count; i++) {
        const int idx = (state->cmd_history_start + i) % CMD_HISTORY_SIZE;
        Command *cmd = &state->cmd_history[idx];

        mvwprintw(state->command_win, start_row + i, 0, "~$ %s ", cmd->command);

        wattron(state->command_win, COLOR_PAIR(1));
        wprintw(state->command_win, "[%.1fs]", cmd->time_taken);
        wattroff(state->command_win, COLOR_PAIR(1));

        wprintw(state->command_win, " [");
        if (cmd->status) {
            wattron(state->command_win, COLOR_PAIR(2));
            wprintw(state->command_win, "ok");
            wattroff(state->command_win, COLOR_PAIR(2));
        } else {
            wattron(state->command_win, COLOR_PAIR(3));
            wprintw(state->command_win, "error: %s", cmd->error_msg);
            wattroff(state->command_win, COLOR_PAIR(3));
        }
        wprintw(state->command_win, "]");
    }
    if (state->mode == COMMAND_MODE) {
        wattron(state->command_win, COLOR_PAIR(4));
        mvwprintw(state->command_win, CMD_HISTORY_SIZE, 0, "~$ ");
        wattroff(state->command_win, COLOR_PAIR(4));
        wprintw(state->command_win, "%s", state->command_input);
    }
    wrefresh(state->command_win);
}

void debugPrint(char* format, ...) {
    if (DEBUG_GUI) {
        va_list args;
        va_start(args, format);
        wclear(state->debug_win);
        vw_printw(state->debug_win, format, args);
        va_end(args);
        wrefresh(state->debug_win);
    }
}

void draw() {
    draw_grid();
    draw_state();
    draw_history();
}