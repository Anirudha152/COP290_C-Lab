#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "primary_storage.h"

#define TOTAL_ROWS 999
#define TOTAL_COLS 18276
#define MIN_CELL_WIDTH 4
#define MAX_CELL_WIDTH 12
#define CMD_BUFFER_SIZE 256
#define INPUT_BUFFER_SIZE 64
#define SCROLL_AMOUNT 10
#define MAX_COL_LABEL 4

typedef struct
{
    short start_row;
    short start_col;
    short visible_rows;
    short visible_cols;
    short cell_width;
} Viewport;

typedef struct
{
    short row;
    short col;
    clock_t time;
} LastEdit;

typedef struct
{
    WINDOW *main_win;
    WINDOW *cmd_win;
    short curr_row;
    short curr_col;
    short mode;
    char cmd_buffer[CMD_BUFFER_SIZE];
    short cmd_pos;
    double last_cmd_time;
    LastEdit last_edit;
    Viewport viewport;
} SpreadsheetState;

typedef enum
{
    GRID_MODE,
    COMMAND_MODE
} Mode;

SpreadsheetState *init_spreadsheet()
{
    SpreadsheetState *state = malloc(sizeof(SpreadsheetState));
    if (!state)
        return NULL;

    initStorage(TOTAL_ROWS, TOTAL_COLS);

    short max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    state->main_win = newwin(max_y - 3, max_x, 0, 0);
    state->cmd_win = newwin(3, max_x, max_y - 3, 0);
    keypad(state->main_win, TRUE);
    keypad(state->cmd_win, TRUE);

    state->curr_row = 0;
    state->curr_col = 0;
    state->mode = GRID_MODE;
    state->cmd_pos = 0;
    state->cmd_buffer[0] = '\0';
    state->last_cmd_time = 0.0;
    state->last_edit = (LastEdit){-1, -1, 0};
    state->viewport = (Viewport){
        .start_row = 0,
        .start_col = 0,
        .visible_rows = 10,
        .visible_cols = 10,
        .cell_width = 8};

    return state;
}

void cleanup_spreadsheet(SpreadsheetState *state)
{
    if (state)
    {
        delwin(state->main_win);
        delwin(state->cmd_win);
        free(state);
    }
    endwin();
}

void index_to_excel_col(const short col, char *buffer)
{
    int len = 0;
    if (col >= 26)
    {
        const int first = col / 26 - 1;
        const int second = col % 26;
        if (first >= 26)
        {
            buffer[len++] = (char)('A' + (first / 26 - 1));
            buffer[len++] = (char)('A' + (first % 26));
        }
        else
        {
            buffer[len++] = (char)('A' + first);
        }
        buffer[len++] = (char)('A' + second);
    }
    else
    {
        buffer[len++] = (char)('A' + col);
    }
    buffer[len] = '\0';
}

short excel_col_to_index(const char *col)
{
    short result = 0;
    const unsigned int len = strlen(col);
    for (int i = 0; i < len - 1; i++)
    {
        result = (result + (col[i] - 'A' + 1)) * 26;
    }
    result += col[len - 1] - 'A';
    return result;
}

void move_cursor(SpreadsheetState *state, const short delta_row, const short delta_col)
{
    const short new_row = state->curr_row + delta_row;
    const short new_col = state->curr_col + delta_col;

    if (new_row >= 0 && new_row < TOTAL_ROWS)
    {
        state->curr_row = new_row;
        if (new_row < state->viewport.start_row)
            state->viewport.start_row = new_row;
        else if (new_row >= state->viewport.start_row + state->viewport.visible_rows)
            state->viewport.start_row = new_row - state->viewport.visible_rows + 1;
    }

    if (new_col >= 0 && new_col < TOTAL_COLS)
    {
        state->curr_col = new_col;
        if (new_col < state->viewport.start_col)
            state->viewport.start_col = new_col;
        else if (new_col >= state->viewport.start_col + state->viewport.visible_cols)
            state->viewport.start_col = new_col - state->viewport.visible_cols + 1;
    }
}

void scroll_viewport(SpreadsheetState *state, const short delta_row, const short delta_col)
{
    const short new_start_row = state->viewport.start_row + delta_row;
    const short new_start_col = state->viewport.start_col + delta_col;

    if (new_start_row >= 0 && new_start_row <= TOTAL_ROWS - state->viewport.visible_rows)
        state->viewport.start_row = new_start_row;

    if (new_start_col >= 0 && new_start_col <= TOTAL_COLS - state->viewport.visible_cols)
        state->viewport.start_col = new_start_col;
}

void resize_cells(SpreadsheetState *state, const short delta)
{
    const short new_width = state->viewport.cell_width + delta;
    if (new_width >= MIN_CELL_WIDTH && new_width <= MAX_CELL_WIDTH)
    {
        state->viewport.cell_width = new_width;
    }
}

void process_expression(SpreadsheetState *state, const char *input, const short row, const short col)
{
    const clock_t start = clock();

    const int value = atoi(input);
    setValue(row, col, value);
    setState(row, col, 0);

    state->last_edit.row = row;
    state->last_edit.col = col;
    state->last_edit.time = clock();

    state->last_cmd_time = ((double)(clock() - start)) / CLOCKS_PER_SEC * 1000;
}

void handle_movement_command(SpreadsheetState *state, const char cmd)
{
    switch (cmd)
    {
    case 'w':
    case 'W':
        scroll_viewport(state, -SCROLL_AMOUNT, 0);
        break;
    case 's':
    case 'S':
        scroll_viewport(state, SCROLL_AMOUNT, 0);
        break;
    case 'a':
    case 'A':
        scroll_viewport(state, 0, -SCROLL_AMOUNT);
        break;
    case 'd':
    case 'D':
        scroll_viewport(state, 0, SCROLL_AMOUNT);
        break;
    default:
        break;
    }
}

void parse_cell_reference(const char *ref, short *row, short *col)
{
    char col_str[MAX_COL_LABEL] = {0};
    int i = 0;

    while (isalpha(ref[i]) && i < MAX_COL_LABEL - 1)
    {
        col_str[i] = (char)toupper(ref[i]);
        i++;
    }
    col_str[i] = '\0';

    *col = excel_col_to_index(col_str);
    *row = atoi(ref + i) - 1;
}

void process_command(SpreadsheetState *state)
{
    const clock_t start = clock();

    if (strlen(state->cmd_buffer) == 1 && strchr("wasdWASD", state->cmd_buffer[0]))
    {
        handle_movement_command(state, state->cmd_buffer[0]);
    }
    else
    {
        char *equals = strchr(state->cmd_buffer, '=');
        if (equals)
        {
            *equals = '\0';
            short row, col;
            parse_cell_reference(state->cmd_buffer, &row, &col);
            if (row >= 0 && row < TOTAL_ROWS && col >= 0 && col < TOTAL_COLS)
            {
                process_expression(state, equals + 1, row, col);
            }
            *equals = '=';
        }
    }

    state->last_cmd_time = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;
    state->cmd_buffer[0] = '\0';
    state->cmd_pos = 0;
}

void draw_cell(WINDOW *win, const int value, const short y, const short x, const short width, const int attrs)
{
    if (attrs)
        wattron(win, attrs);
    mvwprintw(win, y, x, "%-*d", width - 1, value);
    if (attrs)
        wattroff(win, attrs);
}

void draw_grid(const SpreadsheetState *state)
{
    wclear(state->main_win);

    for (short j = 0; j < state->viewport.visible_cols; j++)
    {
        const short actual_col = j + state->viewport.start_col;
        char col_label[MAX_COL_LABEL];
        index_to_excel_col(actual_col, col_label);
        mvwprintw(state->main_win, 0, (j + 1) * state->viewport.cell_width, "%s", col_label);
    }

    for (short i = 0; i < state->viewport.visible_rows; i++)
    {
        const short actual_row = i + state->viewport.start_row;
        mvwprintw(state->main_win, i + 1, 0, "%4d", actual_row + 1);

        for (short j = 0; j < state->viewport.visible_cols; j++)
        {
            const short actual_col = j + state->viewport.start_col;
            const short x = (j + 1) * state->viewport.cell_width;
            const short y = i + 1;
            const int value = cellValue(actual_row, actual_col);

            int attrs = 0;
            if (state->mode == GRID_MODE && actual_row == state->curr_row && actual_col == state->curr_col)
                attrs |= A_REVERSE;
            if (actual_row == state->last_edit.row && actual_col == state->last_edit.col)
                attrs |= COLOR_PAIR(1);

            draw_cell(state->main_win, value, y, x, state->viewport.cell_width, attrs);
        }
    }

    char curr_col_label[MAX_COL_LABEL];
    index_to_excel_col(state->curr_col, curr_col_label);
    mvwprintw(state->main_win, state->viewport.visible_rows + 2, 0,
              "Cell: %s%d | Mode: %s | Last cmd: %.1fms | Tab: switch mode | Enter: edit | Backspace: clear",
              curr_col_label, state->curr_row + 1,
              state->mode == GRID_MODE ? "Grid" : "Command",
              state->last_cmd_time);

    wrefresh(state->main_win);
}

void draw_command_line(SpreadsheetState *state)
{
    wclear(state->cmd_win);
    mvwprintw(state->cmd_win, 0, 0, "[%.1f] > %s", state->last_cmd_time, state->cmd_buffer);
    wrefresh(state->cmd_win);
}

void handle_grid_input(SpreadsheetState *state, const int ch)
{
    switch (ch)
    {
    case '\t':
        state->mode = COMMAND_MODE;
        break;
    case 'w':
    case 'W':
        move_cursor(state, -1, 0);
        break;
    case 's':
    case 'S':
        move_cursor(state, 1, 0);
        break;
    case 'a':
    case 'A':
        move_cursor(state, 0, -1);
        break;
    case 'd':
    case 'D':
        move_cursor(state, 0, 1);
        break;
    case '+':
        resize_cells(state, 1);
        break;
    case '-':
        resize_cells(state, -1);
        break;
    case '\n':
    {
        char input[INPUT_BUFFER_SIZE] = {0};
        echo();
        mvwprintw(state->main_win, state->viewport.visible_rows + 3, 0, "Enter value: ");
        wrefresh(state->main_win);
        wgetnstr(state->main_win, input, INPUT_BUFFER_SIZE - 1);
        noecho();
        process_expression(state, input, state->curr_row, state->curr_col);
        break;
    }
    case KEY_BACKSPACE:
    case 127:
        setValue(state->curr_row, state->curr_col, 0);
        setState(state->curr_row, state->curr_col, 0);
        state->last_edit.row = state->curr_row;
        state->last_edit.col = state->curr_col;
        state->last_edit.time = clock();
        break;
    default:
        break;
    }
}

void handle_command_input(SpreadsheetState *state, const int ch)
{
    switch (ch)
    {
    case '\t':
        state->mode = GRID_MODE;
        break;
    case '\n':
        process_command(state);
        break;
    case KEY_BACKSPACE:
    case 127:
        if (state->cmd_pos > 0)
        {
            state->cmd_buffer[--state->cmd_pos] = '\0';
        }
        break;
    default:
        if (state->cmd_pos < CMD_BUFFER_SIZE - 1 && ch >= 32 && ch <= 126)
        {
            state->cmd_buffer[state->cmd_pos++] = (char)ch;
            state->cmd_buffer[state->cmd_pos] = '\0';
        }
        break;
    }
}

int main()
{
    initscr();
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    SpreadsheetState *state = init_spreadsheet();
    if (!state)
    {
        endwin();
        fprintf(stderr, "Failed to initialize spreadsheet\n");
        return 1;
    }

    while (1)
    {
        draw_grid(state);
        draw_command_line(state);

        const int ch = getch();
        if (ch == 'q' || ch == 'Q')
            break;

        if (state->mode == GRID_MODE)
        {
            handle_grid_input(state, ch);
        }
        else
        {
            handle_command_input(state, ch);
        }
    }

    cleanup_spreadsheet(state);
    return 0;
}
