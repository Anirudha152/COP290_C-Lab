#ifndef CONSTANTS_H
#define CONSTANTS_H
#define MIN_CELL_WIDTH 5
#define MAX_CELL_WIDTH 12
#define DEFAULT_CELL_WIDTH 8
#define CMD_BUFFER_SIZE 256
#define INPUT_BUFFER_SIZE 64
#define SCROLL_AMOUNT 4
#define MAX_COL_LABEL 4
#define CMD_HISTORY_SIZE 5
#define VIEWPORT_ROWS 10
#define DEBUG_GUI 1
#define GUI 0
#include <time.h>
short max(short a, short b);
short min(short a, short b);
void remove_spaces(char *s);
void to_upper(char *s);
int count_char(const char *s, char c);
double sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td);
#endif
