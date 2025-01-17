#include "constants.h"
#include <string.h>

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
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