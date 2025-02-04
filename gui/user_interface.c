#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "../constants.h"
#include "../parsing/cell_indexing.h"
#include "../backend/compute_unit.h"
#include "draw.h"
#include "user_interface.h"


void add_to_history(const Command com) {
    const int index = (state->cmd_history_start + state->cmd_history_count) % CMD_HISTORY_SIZE;

    if (state->cmd_history_count < CMD_HISTORY_SIZE) {
        state->cmd_history_count++;
    } else {
        state->cmd_history_start = (state->cmd_history_start + 1) % CMD_HISTORY_SIZE;
    }

    strncpy(state->cmd_history[index].command, com.command, CMD_BUFFER_SIZE - 1);
    state->cmd_history[index].time_taken = com.time_taken;
    state->cmd_history[index].status = com.status;
    if (com.error_msg[0] != '\0') {
        strncpy(state->cmd_history[index].error_msg, com.error_msg, 63);
    } else {
        state->cmd_history[index].error_msg[0] = '\0';
    }
}

void init_spreadsheet() {
    state = malloc(sizeof(DisplayState));
    if (!state) return;

    short max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    state->grid_win = newwin(VIEWPORT_ROWS + 4, max_x, 0, 0);
    state->status_win = newwin(7, max_x, VIEWPORT_ROWS + 4, 0);
    state->command_win = newwin(CMD_HISTORY_SIZE + 1, max_x, max_y - (CMD_HISTORY_SIZE + 1), 0);
    state->debug_win = newwin(5, max_x, VIEWPORT_ROWS + 12, 0);

    keypad(state->grid_win, TRUE);
    keypad(state->command_win, TRUE);

    state->curr_row = 0;
    state->curr_col = 0;
    state->mode = INTERACTIVE_MODE;
    state->cmd_pos = 0;
    state->command_input[0] = '\0';
    state->viewport = (Viewport) {
        .start_row = 0,
        .start_col = 0,
        .visible_rows = VIEWPORT_ROWS,
        .visible_cols = VIEWPORT_ROWS,
        .cell_width = 8
    };
    state->cmd_history_count = 0;
    state->cmd_history_start = 0;
}

void cleanup_spreadsheet() {
    if (state) {
        delwin(state->grid_win);
        delwin(state->status_win);
        delwin(state->command_win);
        delwin(state->debug_win);
        free(state);
    }
    endwin();
}

void move_cursor(const short delta_row, const short delta_col) {
    const short new_row = max(min(state->curr_row + delta_row, tot_rows - 1), 0);
    const short new_col = max(min(state->curr_col + delta_col, tot_cols - 1), 0);
    if (new_row < state->viewport.start_row) {
        state->viewport.start_row = new_row;
    } else if (new_row >= state->viewport.start_row + state->viewport.visible_rows) {
        state->viewport.start_row = new_row - state->viewport.visible_rows + 1;
    }
    if (new_col < state->viewport.start_col) {
        state->viewport.start_col = new_col;
    } else if (new_col >= state->viewport.start_col + state->viewport.visible_cols) {
        state->viewport.start_col = new_col - state->viewport.visible_cols + 1;
    }
    state->curr_row = new_row;
    state->curr_col = new_col;
}

void move_viewport(const short delta_row, const short delta_col) {
    const short new_start_row = max(min(state->viewport.start_row + delta_row, tot_rows - state->viewport.visible_rows), 0);
    const short new_start_col = max(min(state->viewport.start_col + delta_col, tot_cols - state->viewport.visible_cols), 0);
    state->viewport.start_row = new_start_row;
    state->viewport.start_col = new_start_col;
}

void resize_cells(const short delta) {
    state->viewport.cell_width = max(min(state->viewport.cell_width + delta, MAX_CELL_WIDTH), MIN_CELL_WIDTH);
}

void handle_movement_command(const char cmd) {
    switch (cmd) {
        case 'w':
        case 'W':
            move_viewport(-SCROLL_AMOUNT, 0);
        break;
        case 's':
        case 'S':
            move_viewport(SCROLL_AMOUNT, 0);
        break;
        case 'a':
        case 'A':
            move_viewport(0, -SCROLL_AMOUNT);
        break;
        case 'd':
        case 'D':
            move_viewport(0, SCROLL_AMOUNT);
        break;
        default:
            break;
    }
}

void process_cli_input() {
    char command[CMD_BUFFER_SIZE];
    char error_msg[64];
    Command com;
    strcpy(command, state->command_input);
    if (strlen(state->command_input) == 1 && strchr("wasd", state->command_input[0])) {
        handle_movement_command(state->command_input[0]);
    } else if (strlen(state->command_input) == 1 && strchr("q", state->command_input[0])) {
        cleanup_spreadsheet(state);
    } else if (!abs(strcmp(command, "disable_output"))) {
        // disable output here
    } else if (!abs(strcmp(command, "enable_output"))) {
        // enable output here
    } else if (strncmp(command, "scroll_to", 9) == 0) {
        if (command[9] != ' ') {
            strcpy(error_msg, "Invalid scroll argument");
            goto error;
        }
        short row, col;
        const int resp = parse_cell_reference(command + 10, &row, &col);
        if (!resp) {
            strcpy(error_msg, "Invalid scroll argument");
            goto error;
        }
        if (row >= 0 && row < tot_rows && col >= 0 && col < tot_cols) {
            state->curr_row = row;
            state->curr_col = col;
            state->viewport.start_row = max(min(row - state->viewport.visible_rows / 2, tot_rows - state->viewport.visible_rows), 0);
            state->viewport.start_col = max(min(col - state->viewport.visible_cols / 2, tot_cols - state->viewport.visible_cols), 0);
        } else {
            strcpy(error_msg, "Cell Reference Out of Bounds");
            goto error;
        }
    } else {
        char *equals = strchr(state->command_input, '=');
        const int equal_count = count_char(state->command_input, '=');
        if (equals && equal_count == 1) {
            const int equal_pos = equals - state->command_input;
            *equals = '\0';
            short row, col;
            const int resp = parse_cell_reference(state->command_input, &row, &col);
            if (!resp || equal_pos != resp) {
                strcpy(error_msg, "Invalid Expression");
                goto error;
            }
            if (row >= 0 && row < tot_rows && col >= 0 && col < tot_cols) {
                com = process_expression(command, state->viewport.start_row, state->viewport.start_col);
                add_to_history(com);
            } else {
                strcpy(error_msg, "Cell Reference Out of Bounds");
                goto error;
            }
        } else {
            strcpy(error_msg, "Unrecognized Command");
            goto error;
        }
    }
    state->command_input[0] = '\0';
    state->cmd_pos = 0;
    return;
    error:
        com.status = 0;
        com.time_taken = 0.0;

        strcpy(com.error_msg, error_msg);
        strcpy(com.command, command);
        add_to_history(com);
}

void handle_interactive_input(const int ch) {
    switch (ch) {
        case '\t':
            state->mode = COMMAND_MODE;
        break;
        case 'w':
            move_cursor(-1, 0);
        break;
        case 's':
            move_cursor(1, 0);
        break;
        case 'a':
            move_cursor(0, -1);
        break;
        case 'd':
            move_cursor(0, 1);
        break;
        case 'W':
            move_viewport(-1, 0);
        break;
        case 'S':
            move_viewport(1, 0);
        break;
        case 'A':
            move_viewport(0, -1);
        break;
        case 'D':
            move_viewport(0, 1);
        break;
        case '+':
            resize_cells(1);
        break;
        case '-':
            resize_cells(-1);
        break;
        case '\n': {
            char input[INPUT_BUFFER_SIZE] = {0};
            echo();
            mvwprintw(state->grid_win, 12, 0, "[");
            wattron(state->grid_win, COLOR_PAIR(4));
            wprintw(state->grid_win, "Expression");
            wattroff(state->grid_win, COLOR_PAIR(4));
            wprintw(state->grid_win, "]> ");
            wrefresh(state->grid_win);
            wgetnstr(state->grid_win, input, INPUT_BUFFER_SIZE - 1);
            noecho();
            char command[CMD_BUFFER_SIZE];
            char col_label[MAX_COL_LABEL];
            col_index_to_label(state->curr_col, col_label);
            sprintf(command, "%s%d=%s", col_label, state->curr_row + 1, input);
            strcpy(state->command_input, command);
            process_cli_input();
            break;
        }
        case KEY_BACKSPACE:
        case 127: {
            char col_label[MAX_COL_LABEL];
            char command[CMD_BUFFER_SIZE];
            col_index_to_label(state->curr_col, col_label);
            sprintf(command, "%s%d=0", col_label, state->curr_row + 1);
            strcpy(state->command_input, command);
            process_cli_input();
            break;
        }
        default:
            break;
    }
}

void handle_command_input(const int ch) {
    switch (ch) {
        case '\t':
            state->mode = INTERACTIVE_MODE;
            break;
        case '\n':
            process_cli_input();
            break;
        case KEY_BACKSPACE:
        case 127:
            if (state->cmd_pos > 0) {
                state->command_input[--state->cmd_pos] = '\0';
            }
            break;
        default:
            if (state->cmd_pos < CMD_BUFFER_SIZE - 1 && ch >= 32 && ch <= 126) {
                state->command_input[state->cmd_pos++] = (char) ch;
                state->command_input[state->cmd_pos] = '\0';
            }
            break;
    }
}

void initialize() {
    initscr();
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_spreadsheet();
    if (!state) {
        endwin();
        fprintf(stderr, "Failed to initialize spreadsheet\n");
    }
}

void run_user_interface() {
    initialize();
    while (1) {
        draw();
        const int ch = getch();
        if (ch == 'q' || ch == 'Q')
            break;
        // MAJOR BUG HERE, IF 'q' IS ENTERED ON KEYBOARD PROGRAM FUCKING DIES

        if (state->mode == INTERACTIVE_MODE) {
            handle_interactive_input(ch);
        } else {
            handle_command_input(ch);
        }
    }
    cleanup_spreadsheet(state);
}