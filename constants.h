#ifndef CONSTANTS_H
#define CONSTANTS_H
#define MIN_CELL_WIDTH 5
#define MAX_CELL_WIDTH 12
#define CMD_BUFFER_SIZE 256
#define INPUT_BUFFER_SIZE 64
#define SCROLL_AMOUNT 10
#define MAX_COL_LABEL 4
#define CMD_HISTORY_SIZE 5
#define VIEWPORT_ROWS 10
#define GUI 0
short max(short a, short b);
short min(short a, short b);
void remove_spaces(char *s);
void to_upper(char *s);
int count_char(const char *s, char c);
#endif
