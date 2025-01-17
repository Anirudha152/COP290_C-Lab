#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include "primary_storage.h"
#include "compute_unit.h"

#define TOTAL_ROWS 999
#define TOTAL_COLS 18276
#define MIN_CELL_WIDTH 5
#define MAX_CELL_WIDTH 12
#define CMD_BUFFER_SIZE 256
#define INPUT_BUFFER_SIZE 64
#define SCROLL_AMOUNT 10
#define MAX_COL_LABEL 4
#define CMD_HISTORY_SIZE 5
#define VIEWPORT_ROWS 10

typedef struct {
    char command[CMD_BUFFER_SIZE];
    double time_taken;
    int success;
    char error_msg[64];
} CommandHistory;

typedef struct {
    short start_row;
    short start_col;
    short visible_rows;
    short visible_cols;
    short cell_width;
} Viewport;

typedef struct {
    short row;
    short col;
    clock_t time;
} LastEdit;

typedef enum {
    INTERACTIVE_MODE,
    COMMAND_MODE
} Mode;

typedef struct {
    WINDOW *grid_win;
    WINDOW *status_win;
    WINDOW *command_win;
    WINDOW *debug_win;
    short curr_row;
    short curr_col;
    Mode mode;
    char command_input[CMD_BUFFER_SIZE];
    short cmd_pos;
    double last_cmd_time;
    LastEdit last_edit;
    Viewport viewport;
    CommandHistory cmd_history[CMD_HISTORY_SIZE];
    int cmd_history_count;
    int cmd_history_start;
} DisplayState;

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

void add_to_history(DisplayState *state, const char *cmd, double time_taken, int success, const char *error_msg) {
    int index = (state->cmd_history_start + state->cmd_history_count) % CMD_HISTORY_SIZE;

    if (state->cmd_history_count < CMD_HISTORY_SIZE) {
        state->cmd_history_count++;
    } else {
        state->cmd_history_start = (state->cmd_history_start + 1) % CMD_HISTORY_SIZE;
    }

    strncpy(state->cmd_history[index].command, cmd, CMD_BUFFER_SIZE - 1);
    state->cmd_history[index].time_taken = time_taken;
    state->cmd_history[index].success = success;
    if (error_msg) {
        strncpy(state->cmd_history[index].error_msg, error_msg, 63);
    } else {
        state->cmd_history[index].error_msg[0] = '\0';
    }
}

void update_cell(WINDOW *win, int value, short y, short x, short width, int attrs) {
    wmove(win, y, x);
    wclrtoeol(win);
    if (attrs) wattron(win, attrs);
    wprintw(win, "%-*d", width - 1, value);
    if (attrs) wattroff(win, attrs);
    wrefresh(win);
}

void draw_history(DisplayState *state) {
    wclear(state->command_win);

    // Calculate starting position to ensure input is at bottom
    int start_row = CMD_HISTORY_SIZE - state->cmd_history_count;

    // Draw history entries starting from calculated position
    for (int i = 0; i < state->cmd_history_count; i++) {
        int idx = (state->cmd_history_start + i) % CMD_HISTORY_SIZE;
        CommandHistory *cmd = &state->cmd_history[idx];

        mvwprintw(state->command_win, start_row + i, 0, "~$ %s ", cmd->command);

        wattron(state->command_win, COLOR_PAIR(1));
        wprintw(state->command_win, "[%.1fs]", cmd->time_taken);
        wattroff(state->command_win, COLOR_PAIR(1));

        wprintw(state->command_win, " [");
        if (cmd->success) {
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

DisplayState *init_spreadsheet() {
    DisplayState *state = malloc(sizeof(DisplayState));
    if (!state) return NULL;

    initStorage(TOTAL_ROWS, TOTAL_COLS);

    short max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Adjust window positions to put command window at bottom
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
    state->last_cmd_time = 0.0;
    state->last_edit = (LastEdit){-1, -1, 0};
    state->viewport = (Viewport){
        .start_row = 0,
        .start_col = 0,
        .visible_rows = 10,
        .visible_cols = 10,
        .cell_width = 8
    };
    state->cmd_history_count = 0;
    state->cmd_history_start = 0;

    return state;
}

//prints to debug win printf style
void debugPrint(DisplayState *state, char* format, ...) {
    va_list args;
    va_start(args, format);
    wclear(state->debug_win);
    vw_printw(state->debug_win, format, args);
    va_end(args);
    wrefresh(state->debug_win);
}

void cleanup_spreadsheet(DisplayState *state) {
    if (state) {
        delwin(state->grid_win);
        delwin(state->status_win);
        delwin(state->command_win);
        free(state);
    }
    endwin();
}

void col_index_to_label(const short col, char *buffer) {
    int len = 0;
    if (col >= 26) {
        const int first = col / 26 - 1;
        const int second = col % 26;
        if (first >= 26) {
            buffer[len++] = (char) ('A' + (first / 26 - 1));
            buffer[len++] = (char) ('A' + (first % 26));
        } else {
            buffer[len++] = (char) ('A' + first);
        }
        buffer[len++] = (char) ('A' + second);
    } else {
        buffer[len++] = (char) ('A' + col);
    }
    buffer[len] = '\0';
}

short label_to_col_index(const char *col) {
    short result = 0;
    const unsigned int len = strlen(col);
    for (int i = 0; i < len - 1; i++) {
        result = (result + (col[i] - 'A' + 1)) * 26;
    }
    result += col[len - 1] - 'A';
    return result;
}

bool cellWithinExpression(Expression* expr, const short row, const short col) {
    if (expr->type == 0) {
        if (expr->value1.type == 1 && expr->value1.cell->row == row && expr->value1.cell->col == col)
            return true;
        return false;
    }
    if (expr->type == 1) {
        if (expr->value1.type == 1 && expr->value1.cell->row == row && expr->value1.cell->col == col) {
            return true;
        }
        if (expr->value2.type == 1 && expr->value2.cell->row == row && expr->value2.cell->col == col) {
            return true;
        }
        return false;
    }
    if (expr->type == 2) {
        return expr->function.range.start_row <= row && expr->function.range.end_row >= row &&
               expr->function.range.start_col <= col && expr->function.range.end_col >= col;
    }
    return false;
}

void move_cursor(DisplayState *state, const short delta_row, const short delta_col) {
    const short new_row = max(min(state->curr_row + delta_row, TOTAL_ROWS - 1), 0);
    const short new_col = max(min(state->curr_col + delta_col, TOTAL_COLS - 1), 0);
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

void move_viewport(DisplayState *state, const short delta_row, const short delta_col) {
    const short new_start_row = max(min(state->viewport.start_row + delta_row, TOTAL_ROWS - state->viewport.visible_rows), 0);
    const short new_start_col = max(min(state->viewport.start_col + delta_col, TOTAL_COLS - state->viewport.visible_cols), 0);
    state->viewport.start_row = new_start_row;
    state->viewport.start_col = new_start_col;
}

void resize_cells(DisplayState *state, const short delta) {
    state->viewport.cell_width = max(min(state->viewport.cell_width + delta, MAX_CELL_WIDTH), MIN_CELL_WIDTH);
}

void parse_cell_reference(const char *ref, short *row, short *col) {
    char col_str[MAX_COL_LABEL] = {0};
    int i = 0;

    while (isalpha(ref[i]) && i < MAX_COL_LABEL - 1) {
        col_str[i] = (char) toupper(ref[i]);
        i++;
    }
    col_str[i] = '\0';

    *col = label_to_col_index(col_str);
    *row = atoi(ref + i) - 1;
}

Value parse_value(DisplayState* state, const char *expr, char **end)  {
    Value val = {0, 0, NULL};
    while (*expr == ' ') expr++;

    if (isdigit(*expr) || *expr == '-') {
        val.type = 0;
        val.value = strtol(expr, end, 10);
        return val;
    }

    if (isalpha(*expr)) {
        val.type = 1;
        short row, col;
        parse_cell_reference(expr, &row, &col);

        val.cell = getCell(row, col);

        *end = (char*)expr + strcspn(expr, "+-*/(),:) ");
        return val;
    }

    *end = (char*)expr;
    return val;
}

Range parse_range(const char *expr, char **end) {
    Range range = {0, 0, 0, 0, 0};
    char *colon = strchr(expr, ':');
    if (!colon) {
        *end = (char*)expr;
        return range;
    }

    short start_row, start_col, end_row, end_col;
    parse_cell_reference(expr, &start_row, &start_col);
    parse_cell_reference(colon + 1, &end_row, &end_col);

    if (start_row > end_row || start_col > end_col) {
        *end = (char*)expr;
        return range;
    }

    range.start_row = start_row;
    range.start_col = start_col;
    range.end_row = end_row;
    range.end_col = end_col;
    range.dimension = (start_row == end_row || start_col == end_col) ? 1 : 2;

    *end = strchr(colon, ')');
    return range;
}

char* get_expression_string(const Expression *expr) {
    static char buffer[CMD_BUFFER_SIZE];
    buffer[0] = '\0';
    if (expr->type == 0) {
        if (expr->value1.type == 0) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d", expr->value1.value);
        } else {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d", col_label, expr->value1.cell->row + 1);
        }
    } else if (expr->type == 1) {
        char op = (expr->operation == 0) ? '+' : (expr->operation == 1) ? '-' : (expr->operation == 2) ? '*' : '/';
        if (expr->value1.type == 0 && expr->value2.type == 0 ) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %d", expr->value1.value, op, expr->value2.value);
        } else if (expr->value1.type == 0 && expr->value2.type == 1) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value2.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %s%d", expr->value1.value, op, col_label, expr->value2.cell->row + 1);
        } else if (expr->value1.type == 1 && expr->value2.type == 0) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %d", col_label, expr->value1.cell->row + 1, op, expr->value2.value);
        } else {
            char col_label[MAX_COL_LABEL];
            char col_label2[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            col_index_to_label(expr->value2.cell->col, col_label2);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %s%d", col_label, expr->value1.cell->row + 1, op, col_label2, expr->value2.cell->row + 1);
        }
    } else if (expr->type == 2) {
        char start_col_label[MAX_COL_LABEL];
        col_index_to_label(expr->function.range.start_col, start_col_label);
        char end_col_label[MAX_COL_LABEL];
        col_index_to_label(expr->function.range.end_col, end_col_label);
        char function_name[6];
        if (expr->function.type == 0) {
            snprintf(function_name, 6, "MIN");
        } else if (expr->function.type == 1) {
            snprintf(function_name, 6, "MAX");
        } else if (expr->function.type == 2) {
            snprintf(function_name, 6, "AVG");
        } else if (expr->function.type == 3) {
            snprintf(function_name, 6, "SUM");
        } else if (expr->function.type == 4) {
            snprintf(function_name, 6, "STDEV");
        }
        snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d:%s%d)", function_name, start_col_label, expr->function.range.start_row + 1, end_col_label, expr->function.range.end_row + 1);
    }

    return buffer;
}

// process expression from interactive mode
void process_expression(DisplayState *state, const char *input, const short row, const short col) {
    clock_t start = clock();
    char *end;
    Value val1, val2;
    Range range;

    while (*input == ' ') input++;

    if (strncmp(input, "MIN(", 4) == 0 || strncmp(input, "MAX(", 4) == 0 ||
        strncmp(input, "AVG(", 4) == 0 || strncmp(input, "SUM(", 4) == 0 ||
        strncmp(input, "STDEV(", 6) == 0 || strncmp(input, "SLEEP(", 6) == 0) {

        short func_type = 0;
        if (strncmp(input, "MIN(", 4) == 0) func_type = 0;
        else if (strncmp(input, "MAX(", 4) == 0) func_type = 1;
        else if (strncmp(input, "AVG(", 4) == 0) func_type = 2;
        else if (strncmp(input, "SUM(", 4) == 0) func_type = 3;
        else if (strncmp(input, "STDEV(", 6) == 0) func_type = 4;
        else if (strncmp(input, "SLEEP(", 6) == 0) func_type = 5;

        const char *range_start = strchr(input, '(') + 1;
        if (func_type == 5) {
            val1 = parse_value(state, range_start, &end);
            setValueExpression(row, col, val1);
        } else {
            range = parse_range(range_start, &end);
            setFunctionExpression(row, col, func_type, range);
        }
    } else {
        val1 = parse_value(state, input, &end);
        while (*end == ' ') end++;
        if (*end == '+' || *end == '-' || *end == '*' || *end == '/') {
            char op = *end++;
            val2 = parse_value(state, end, &end);
            short operation = (op == '+') ? 0 : (op == '-') ? 1 : (op == '*') ? 2 : 3;
            setArithmeticExpression(row, col, val1, val2, operation);
        } else {
            setValueExpression(row, col, val1);
        }
    }

    state->last_edit.row = row;
    state->last_edit.col = col;
    state->last_edit.time = clock();

    double time_taken = ((double)(clock() - start)) / CLOCKS_PER_SEC;
    char cmd_str[CMD_BUFFER_SIZE];
    char col_label[MAX_COL_LABEL];
    col_index_to_label(col, col_label);
    snprintf(cmd_str, CMD_BUFFER_SIZE, "%s%d=%s", col_label, row + 1, input);
    add_to_history(state, cmd_str, time_taken, 1, NULL);
}

void handle_movement_command(DisplayState *state, const char cmd) {
    switch (cmd) {
        case 'w':
        case 'W':
            move_viewport(state, -SCROLL_AMOUNT, 0);
        break;
        case 's':
        case 'S':
            move_viewport(state, SCROLL_AMOUNT, 0);
        break;
        case 'a':
        case 'A':
            move_viewport(state, 0, -SCROLL_AMOUNT);
        break;
        case 'd':
        case 'D':
            move_viewport(state, 0, SCROLL_AMOUNT);
        break;
        default:
            break;
    }
}

void remove_spaces(char *s) {
    char *d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

int count_char(const char *s, const char c) {
    int co = 0;
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == c) {
            co++;
        }
    }
    return co;
}

// process command mode input
void process_command(DisplayState *state) {
    const clock_t start = clock();
    remove_spaces(state->command_input);
    if (strlen(state->command_input) == 1 && strchr("wasdWASD", state->command_input[0])) {
        handle_movement_command(state, state->command_input[0]);
    } else {
        char *equals = strchr(state->command_input, '=');
        const int equal_count = count_char(state->command_input, '=');
        if (equals && equal_count == 1) {
            *equals = '\0';
            short row, col;
            parse_cell_reference(state->command_input, &row, &col);
            if (row >= 0 && row < TOTAL_ROWS && col >= 0 && col < TOTAL_COLS) {
                process_expression(state, equals + 1, row, col);
            }
            *equals = '=';
        }
    }

    state->last_cmd_time = (double) (clock() - start) / CLOCKS_PER_SEC * 1000;
    state->command_input[0] = '\0';
    state->cmd_pos = 0;
}

void draw_cell(WINDOW *win, const int value, const short y, const short x, const short width, const int printmode) {
    if (printmode == 1)
        wattron(win, A_REVERSE);
    else if (printmode == 2)
        wattron(win, COLOR_PAIR(1));
    else if (printmode == 3) {
        wattron(win, A_REVERSE);
        wattron(win, COLOR_PAIR(1));
    } else if (printmode == 4) {
        wattron(win, COLOR_PAIR(4));
    }
    const short len_val = log(abs(value)) + 1 + (value < 0);
    if (printmode % 2 == 0) {
        if (len_val > width - 1) {
            mvwprintw(win, y, x, "%d..", value/(int)pow(10, len_val - width));
        } else {
            mvwprintw(win, y, x, "%-*d", width - 1, value);
        }
    } else {
        mvwprintw(win, y, x, "%-*d", width - 1, value);
    }
    if (printmode == 1)
        wattroff(win, A_REVERSE);
    else if (printmode == 2)
        wattroff(win, COLOR_PAIR(1));
    else if (printmode == 3) {
        wattroff(win, A_REVERSE);
        wattroff(win, COLOR_PAIR(1));
    } else if (printmode == 4) {
        wattroff(win, COLOR_PAIR(4));
    }
}

void draw_grid(const DisplayState *state) {
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
            const int value = getValue(actual_row, actual_col);
            bool cell_in_expr = cellWithinExpression(&getCell(state->curr_row, state->curr_col)->formula, actual_row, actual_col);
            int param;
            if (state->mode == INTERACTIVE_MODE) {
                if (actual_row == state->curr_row && actual_col == state->curr_col && actual_row == state->last_edit.row && actual_col == state->last_edit.col) {
                    param = 3;
                } else if (actual_row == state->last_edit.row && actual_col == state->last_edit.col) {
                    param = 2;
                } else if (actual_row == state->curr_row && actual_col == state->curr_col) {
                    param = 1;
                } else if (cell_in_expr) {
                    param = 4;
                } else {
                    param = 0;
                }
            } else {
                if (actual_row == state->last_edit.row && actual_col == state->last_edit.col) {
                    param = 2;
                } else {
                    param = 0;
                }
            }
            draw_cell(state->grid_win, value, y, x, state->viewport.cell_width, param);
        }
        // reprint the last edit cell to ensure it's on top
        if (state->last_edit.row >= state->viewport.start_row && state->last_edit.row < state->viewport.start_row + state->viewport.visible_rows &&
            state->last_edit.col >= state->viewport.start_col && state->last_edit.col < state->viewport.start_col + state->viewport.visible_cols) {
            const short x = (state->last_edit.col - state->viewport.start_col + 1) * state->viewport.cell_width;
            const short y = state->last_edit.row - state->viewport.start_row + 1;
            const int value = cellValue(state->last_edit.row, state->last_edit.col);
            if (state->mode == INTERACTIVE_MODE) {
                if (state->last_edit.row == state->curr_row && state->last_edit.col == state->curr_col) {
                    draw_cell(state->grid_win, value, y, x, state->viewport.cell_width, 3);
                } else {
                    draw_cell(state->grid_win, value, y, x, state->viewport.cell_width, 2);
                }
            } else {
                draw_cell(state->grid_win, value, y, x, state->viewport.cell_width, 2);
            }
        }
    }

    char curr_col_label[MAX_COL_LABEL];
    col_index_to_label(state->curr_col, curr_col_label);
    wrefresh(state->grid_win);
}

void draw_state(DisplayState *state) {
    werase(state->status_win);
    char curr_col_label[MAX_COL_LABEL];
    col_index_to_label(state->curr_col, curr_col_label);

    wattron(state->status_win, COLOR_PAIR(2));
    mvwprintw(state->status_win, 0, 0, "Current Cell:    ");
    mvwprintw(state->status_win, 1, 0, "Cell Value:      ");
    mvwprintw(state->status_win, 2, 0, "Cell Expression: ");
    wattroff(state->status_win, COLOR_PAIR(2));

    mvwprintw(state->status_win, 0, 17, "%s%d", curr_col_label, state->curr_row + 1);
    mvwprintw(state->status_win, 1, 17, "%d", cellValue(state->curr_row, state->curr_col));
    mvwprintw(state->status_win, 2, 17, "%s", get_expression_string(&getCell(state->curr_row, state->curr_col)->formula));
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependencies:    ");
    wattroff(state->status_win, COLOR_PAIR(2));
    for (int i = 0; i < getCell(state->curr_row, state->curr_col)->dependency_count; i++) {
        char col_label[MAX_COL_LABEL];
        col_index_to_label(getCell(state->curr_row, state->curr_col)->dependencies[i]->col, col_label);
        bool done = false;
        if (getCell(getCell(state->curr_row, state->curr_col)->dependencies[i]->row, getCell(state->curr_row, state->curr_col)->dependencies[i]->col)->state != 0) {
            done = true;
            wattron(state->status_win, COLOR_PAIR(3));
        }
        wprintw(state->status_win, "%s%d ", col_label, getCell(state->curr_row, state->curr_col)->dependencies[i]->row + 1);
        if (done) {
            wattroff(state->status_win, COLOR_PAIR(3));
        }
    }
    wattron(state->status_win, COLOR_PAIR(2));
    wprintw(state->status_win, "\nDependants:      ");
    wattroff(state->status_win, COLOR_PAIR(2));
    Node *node = getCell(state->curr_row, state->curr_col)->head_dependant;
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

void handle_interactive_input(DisplayState *state, const int ch) {
    switch (ch) {
        case '\t':
            state->mode = COMMAND_MODE;
        break;
        case 'w':
            move_cursor(state, -1, 0);
        break;
        case 's':
            move_cursor(state, 1, 0);
        break;
        case 'a':
            move_cursor(state, 0, -1);
        break;
        case 'd':
            move_cursor(state, 0, 1);
        break;
        case 'W':
            move_viewport(state, -1, 0);
        break;
        case 'S':
            move_viewport(state, 1, 0);
        break;
        case 'A':
            move_viewport(state, 0, -1);
        break;
        case 'D':
            move_viewport(state, 0, 1);
        break;
        case '+':
            resize_cells(state, 1);
        break;
        case '-':
            resize_cells(state, -1);
        break;
        case '\n': {
            char input[INPUT_BUFFER_SIZE] = {0};
            echo();
            mvwprintw(state->grid_win, 12, 0, "Enter expression: ");
            wrefresh(state->grid_win);
            wgetnstr(state->grid_win, input, INPUT_BUFFER_SIZE - 1);
            noecho();
            process_expression(state, input, state->curr_row, state->curr_col);
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

void handle_command_input(DisplayState *state, const int ch) {
    switch (ch) {
        case '\t':
            state->mode = INTERACTIVE_MODE;
            break;
        case '\n':
            process_command(state);
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

int main() {
    initscr();
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    DisplayState *state = init_spreadsheet();
    if (!state) {
        endwin();
        fprintf(stderr, "Failed to initialize spreadsheet\n");
        return 1;
    }
    draw_grid(state);
    draw_state(state);
    draw_history(state);

    while (1) {
        const int ch = getch();
        if (ch == 'q' || ch == 'Q')
            break;

        if (state->mode == INTERACTIVE_MODE) {
            handle_interactive_input(state, ch);
        } else {
            handle_command_input(state, ch);
        }
        draw_grid(state);
        draw_state(state);
        draw_history(state);
    }

    cleanup_spreadsheet(state);
    return 0;
}