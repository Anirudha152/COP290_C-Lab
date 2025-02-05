#ifndef constants_h
#define constants_h
#define MIN_CELL_WIDTH 5
#define MAX_CELL_WIDTH 12
#define DEFAULT_CELL_WIDTH 8
#define CMD_BUFFER_SIZE 256
#define INPUT_BUFFER_SIZE 64
#define MAX_COL_LABEL 4
extern short SCROLL_AMOUNT; // 10
extern short CMD_HISTORY_SIZE; // 7
extern short VIEWPORT_ROWS; // 10
extern short DEBUG_GUI; // 0
extern short GUI; // no default
extern short TOT_ROWS; // no default
extern short TOT_COLS; // no default
extern short LAZY_EVALUATION; // 1
#include <time.h>
short max(short a, short b);
short min(short a, short b);
void remove_spaces(char *s);
void to_upper(char *s);
int count_char(const char *s, char c);
double sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td);
#endif
