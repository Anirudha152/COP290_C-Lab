#ifndef DRAW_H
#define DRAW_H
#include <time.h>

#include "command_processing.h"

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
    // WINDOW *debug_win;
    short curr_row;
    short curr_col;
    Mode mode;
    char command_input[CMD_BUFFER_SIZE];
    short cmd_pos;
    double last_cmd_time;
    LastEdit last_edit;
    Viewport viewport;
    Command cmd_history[CMD_HISTORY_SIZE];
    int cmd_history_count;
    int cmd_history_start;
} DisplayState;

extern DisplayState *state;
void draw();
void debugPrint(char* format, ...);
#endif