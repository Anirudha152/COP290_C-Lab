#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "cell_indexing.h"
#include "../constants.h"
#include "../backend/primary_storage.h"

// bool cellWithinExpression(const Expression* expression, const short row, const short col) {
//     if (expression->type == VALUE) {
//         if (expression->value.type == CELL_REFERENCE && expression->value.cell->row == row && expression->value.cell->col == col)
//             return true;
//         return false;
//     }
//     if (expression->type == ARITHMETIC) {
//         if (expression->arithmetic.value1.type == CELL_REFERENCE && expression->arithmetic.value1.cell->row == row && expression->arithmetic.value1.cell->col == col) {
//             return true;
//         }
//         if (expression->arithmetic.value2.type == CELL_REFERENCE && expression->arithmetic.value2.cell->row == row && expression->arithmetic.value2.cell->col == col) {
//             return true;
//         }
//         return false;
//     }
//     if (expression->type == FUNCTION) {
//         if (expression->function.type != SLEEP) {
//             return expression->function.range.start_row <= row && expression->function.range.end_row >= row &&
//                expression->function.range.start_col <= col && expression->function.range.end_col >= col;
//         }
//         return expression->function.value.type == CELL_REFERENCE && expression->function.value.cell->row == row && expression->function.value.cell->col == col;
//     }
//     return false;
// }

short col_label_to_index(const char *col) {
    short result = 0;
    const unsigned int len = strlen(col);
    for (int i = 0; i < len - 1; i++) {
        result = (result + (col[i] - 'A' + 1)) * 26;
    }
    result += col[len - 1] - 'A';
    return result;
}

void col_index_to_label(short col, char *buffer) {
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

int parse_cell_reference(const char *ref, short *row, short *col) {
    char col_str[MAX_COL_LABEL] = {0};
    int i = 0;
    int col_ = 0;
    while (isalpha(ref[i]) && isupper(ref[i]) && col_ * 26 + (toupper(ref[i]) - 'A' + 1) <= TOT_COLS) {
        col_str[i] = (char) toupper(ref[i]);
        col_ = col_ * 26 + (toupper(ref[i]) - 'A' + 1);
        i++;
    }
    col_str[i] = '\0';
    if (!isdigit(ref[i])) {
        *col = -1;
        *row = -1;
        return 0;
    }

    int row_ = 0;
    int at_least_one_digit = 0;
    while (isdigit(ref[i]) && (at_least_one_digit || ref[i] != '0') && row_ * 10 + (ref[i] - '0') <= TOT_ROWS) {
        row_ = row_ * 10 + (ref[i] - '0');
        at_least_one_digit = 1;
        i++;
    }
    if (!at_least_one_digit) {
        *col = -1;
        *row = -1;
        return 0;
    }
    if (isdigit(ref[i])) {
        *col = -1;
        *row = -1;
        return 0;
    }

    *col = col_label_to_index(col_str);
    *row = row_ - 1;

    return i;
}