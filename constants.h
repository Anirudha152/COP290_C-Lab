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
int max(int a, int b);
int min(int a, int b);
void remove_spaces(char *s);
int count_char(const char *s, const char c);
#endif
