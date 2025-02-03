#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include "../constants.h"
#include "draw.h"

#include "../parsing/cell_indexing.h"
#include "../backend/compute_unit.h"

DisplayState *state = NULL;

void draw_cell(const int value, const short y, const short x, const short width, const int printmode) {
    if (printmode == 1)
        wattron(state->grid_win, A_REVERSE);
    else if (printmode == 2)
        wattron(state->grid_win, COLOR_PAIR(1));
    else if (printmode == 3) {
        wattron(state->grid_win, A_REVERSE);
        wattron(state->grid_win, COLOR_PAIR(1));
    } else if (printmode == 4) {
        wattron(state->grid_win, COLOR_PAIR(4));
    }
    const short len_val = log(abs(value)) + 1 + (value < 0);
    if (printmode % 2 == 0) {
        if (len_val > width - 1) {
            mvwprintw(state->grid_win, y, x, "%d..", value/(int)pow(10, len_val - width));
        } else {
            mvwprintw(state->grid_win, y, x, "%-*d", width - 1, value);
        }
    } else {
        mvwprintw(state->grid_win, y, x, "%-*d", width - 1, value);
    }
    if (printmode == 1)
        wattroff(state->grid_win, A_REVERSE);
    else if (printmode == 2)
        wattroff(state->grid_win, COLOR_PAIR(1));
    else if (printmode == 3) {
        wattroff(state->grid_win, A_REVERSE);
        wattroff(state->grid_win, COLOR_PAIR(1));
    } else if (printmode == 4) {
        wattroff(state->grid_win, COLOR_PAIR(4));
    }
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
        wattron(state->grid_win, COLOR_PAIR(2));
        mvwprintw(state->grid_win, i + 1, 0, "%4d", actual_row + 1);
        wattroff(state->grid_win, COLOR_PAIR(2));
        for (short j = 0; j < state->viewport.visible_cols; j++) {
            const short actual_col = j + state->viewport.start_col;
            const short x = (j + 1) * state->viewport.cell_width;
            const short y = i + 1;
            const int value = get_cell_value(actual_row, actual_col);
            bool cell_in_expr = cellWithinExpression(&get_cell(state->curr_row, state->curr_col)->formula, actual_row, actual_col);
            int param;
            if (state->mode == INTERACTIVE_MODE) {
                if (actual_row == state->curr_row && actual_col == state->curr_col) {
                    param = 1;
                } else if (cell_in_expr) {
                    param = 4;
                } else {
                    param = 0;
                }
            } else {
                param = 0;
            }
            draw_cell(value, y, x, state->viewport.cell_width, param);
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
    mvwprintw(state->status_win, 0, 0, "Current Cell:    ");
    mvwprintw(state->status_win, 1, 0, "Cell Value:      ");
    mvwprintw(state->status_win, 2, 0, "Cell Expression: ");
    wattroff(state->status_win, COLOR_PAIR(2));

    mvwprintw(state->status_win, 0, 17, "%s%d", curr_col_label, state->curr_row + 1);
    mvwprintw(state->status_win, 1, 17, "%d", get_raw_value(state->curr_row, state->curr_col));
    mvwprintw(state->status_win, 2, 17, "%s", get_expression_string(&get_cell(state->curr_row, state->curr_col)->formula));
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependencies:    ");
    wattroff(state->status_win, COLOR_PAIR(2));
    for (int i = 0; i < get_cell(state->curr_row, state->curr_col)->dependency_count; i++) {
        char col_label[MAX_COL_LABEL];
        col_index_to_label(get_cell(state->curr_row, state->curr_col)->dependencies[i]->col, col_label);
        bool done = false;
        if (get_cell(get_cell(state->curr_row, state->curr_col)->dependencies[i]->row, get_cell(state->curr_row, state->curr_col)->dependencies[i]->col)->state != 0) {
            done = true;
            wattron(state->status_win, COLOR_PAIR(3));
        }
        wprintw(state->status_win, "%s%d ", col_label, get_cell(state->curr_row, state->curr_col)->dependencies[i]->row + 1);
        if (done) {
            wattroff(state->status_win, COLOR_PAIR(3));
        }
    }
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependants:      ");
    wattroff(state->status_win, COLOR_PAIR(2));
    const Node *node = get_cell(state->curr_row, state->curr_col)->head_dependant;
    while (node) {
        char col_label[MAX_COL_LABEL];
        col_index_to_label(node->cell->col, col_label);
        bool done = false;
        if (node->cell->state != 0) {
            done = true;
            wattron(state->status_win, COLOR_PAIR(3));
        }
        wprintw(state->status_win, "%s%d ", col_label, node->cell->row + 1);
        if (done) {
            wattroff(state->status_win, COLOR_PAIR(3));
        }
        node = node->next;
    }
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
        // Always draw input prompt at the bottom line
        wattron(state->command_win, COLOR_PAIR(4));  // Purple for input arrow
        mvwprintw(state->command_win, CMD_HISTORY_SIZE, 0, "~$ ");
        wattroff(state->command_win, COLOR_PAIR(4));
        wprintw(state->command_win, "%s", state->command_input);
    }
    wrefresh(state->command_win);
}

void debugPrint(char* format, ...) {
    if (GUI) {
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