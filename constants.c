#include "constants.h"
#include <string.h>
#include <ctype.h>

short max(const short a, const short b) {
    if (a > b) {
        return a;
    }
    return b;
}

short min(const short a, const short b) {
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