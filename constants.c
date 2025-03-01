#include "constants.h"
#include <string.h>
#include <ctype.h>
#include <time.h>

int max(const int a, const int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int min(const int a, const int b) {
    if (a < b) {
       return a;
    }
    return b;
}

void remove_spaces(char *s) {
    const char *d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while ((*s++ = *d++));
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

void to_upper(char *s) {
    for (int i = 0; i < strlen(s); i++) {
        s[i] = (char)toupper(s[i]);
    }
}

double sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td) {
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0) {
        td->tv_nsec += 1000000000;
        td->tv_sec--;
    } else if (td->tv_sec < 0 && td->tv_nsec > 0) {
        td->tv_nsec -= 1000000000;
        td->tv_sec++;
    }
    return (double) td->tv_sec + (double) td->tv_nsec / 1000000000;
}

int rowcol_to_cell_index(const short row, const short col) {
    return row * TOT_COLS + col;
}

short cell_index_to_row(const int cell_index) {
    return cell_index / TOT_COLS;
}

short cell_index_to_col(const int cell_index) {
    return cell_index % TOT_COLS;
}