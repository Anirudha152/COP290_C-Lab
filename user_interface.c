#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "constants.h"
#include "cell_indexing.h"
#include "compute_unit.h"
#include "draw.h"
#include "user_interface.h"

int tot_rows;
int tot_cols;

void add_to_history(const Command *com) {
    int index = (state->cmd_history_start + state->cmd_history_count) % CMD_HISTORY_SIZE;

    if (state->cmd_history_count < CMD_HISTORY_SIZE) {
        state->cmd_history_count++;
    } else {
        state->cmd_history_start = (state->cmd_history_start + 1) % CMD_HISTORY_SIZE;
    }

    strncpy(state->cmd_history[index].command, com->command, CMD_BUFFER_SIZE - 1);
    state->cmd_history[index].time_taken = com->time_taken;
    state->cmd_history[index].status = com->status;
    if (com->error_msg[0] != '\0') {
        strncpy(state->cmd_history[index].error_msg, com->error_msg, 63);
    } else {
        state->cmd_history[index].error_msg[0] = '\0';
    }
}

void init_spreadsheet() {
    state = malloc(sizeof(DisplayState));
    if (!state) return;

    initStorage(tot_rows, tot_cols);

    short max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    state->grid_win = newwin(VIEWPORT_ROWS + 4, max_x, 0, 0);
    state->status_win = newwin(7, max_x, VIEWPORT_ROWS + 4, 0);
    state->command_win = newwin(CMD_HISTORY_SIZE + 1, max_x, max_y - (CMD_HISTORY_SIZE + 1), 0);
    // state->debug_win = newwin(5, max_x, VIEWPORT_ROWS + 12, 0);

    keypad(state->grid_win, TRUE);
    keypad(state->command_win, TRUE);

    state->curr_row = 0;
    state->curr_col = 0;
    state->mode = INTERACTIVE_MODE;
    state->cmd_pos = 0;
    state->command_input[0] = '\0';
    state->last_edit = (LastEdit) {-1, -1, 0};
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

void process_command() {
    char command_str[CMD_BUFFER_SIZE];
    strcpy(command_str, state->command_input);
    to_upper(state->command_input);
    remove_spaces(state->command_input);
    if (strlen(state->command_input) == 1 && strchr("wasdWASD", state->command_input[0])) {
        handle_movement_command(state->command_input[0]);
    } else {
        char *equals = strchr(state->command_input, '=');
        const int equal_count = count_char(state->command_input, '=');
        if (equals && equal_count == 1) {
            int equal_pos = equals - state->command_input;
            *equals = '\0';
            short row, col;
            int resp = parse_cell_reference(state->command_input, &row, &col);
            if (!resp || equal_pos != resp) {
                Command com = {.status = 0, .time_taken = 0.0, .command = "", .error_msg = "Invalid cell reference"};
                strcpy(com.command, command_str);
                add_to_history(&com);
            } else if (row >= 0 && row < tot_rows && col >= 0 && col < tot_cols) {
                Command com = process_expression(equals + 1, row, col, state->viewport.start_row, state->viewport.start_col);
                strcpy(com.command, command_str);
                add_to_history(&com);
            } else {
                Command com = {.status = 0, .time_taken = 0.0, .command = "", .error_msg = "Invalid cell reference"};
                strcpy(com.command, command_str);
                add_to_history(&com);
            }
            *equals = '=';
        } else {
            Command com = {.status = 0, .time_taken = 0.0, .command = "", .error_msg = "Command not recognized"};
            strcpy(com.command, command_str);
            add_to_history(&com);
        }
    }
    state->command_input[0] = '\0';
    state->cmd_pos = 0;
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
            const Command com = process_expression(input, state->curr_row, state->curr_col, state->viewport.start_row, state->viewport.start_col);

            add_to_history(&com);
            break;
        }
        case KEY_BACKSPACE:
        case 127: {
            Value val = {0, 0, NULL};
            setValueExpression(state->curr_row, state->curr_col, val);
            state->last_edit.row = state->curr_row;
            state->last_edit.col = state->curr_col;
            state->last_edit.time = clock();
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
            process_command();
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

void run_user_interface(const short total_rows, const short total_cols) {
    tot_rows = total_rows;
    tot_cols = total_cols;
    initialize();
    while (1) {
        draw();
        const int ch = getch();
        if (ch == 'q' || ch == 'Q')
            break;

        if (state->mode == INTERACTIVE_MODE) {
            handle_interactive_input(ch);
        } else {
            handle_command_input(ch);
        }
    }
    cleanup_spreadsheet(state);
}