#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "cell_indexing.h"
#include "constants.h"
#include "primary_storage.h"

bool cellWithinExpression(const Expression* expr, const short row, const short col) {
    if (expr->type == 0) {
        if (expr->value1.type == 1 && expr->value1.cell->row == row && expr->value1.cell->col == col)
            return true;
        return false;
    }
    if (expr->type == 1) {
        if (expr->value1.type == 1 && expr->value1.cell->row == row && expr->value1.cell->col == col) {
            return true;
        }
        if (expr->value2.type == 1 && expr->value2.cell->row == row && expr->value2.cell->col == col) {
            return true;
        }
        return false;
    }
    if (expr->type == 2) {
        return expr->function.range.start_row <= row && expr->function.range.end_row >= row &&
               expr->function.range.start_col <= col && expr->function.range.end_col >= col;
    }
    return false;
}

short col_label_to_index(const char *col) {
    short result = 0;
    const unsigned int len = strlen(col);
    for (int i = 0; i < len - 1; i++) {
        result = (result + (col[i] - 'A' + 1)) * 26;
    }
    result += col[len - 1] - 'A';
    return result;
}

void col_index_to_label(const short col, char *buffer) {
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

void parse_cell_reference(const char *ref, short *row, short *col) {
    char col_str[MAX_COL_LABEL] = {0};
    int i = 0;

    while (isalpha(ref[i]) && i < MAX_COL_LABEL - 1) {
        col_str[i] = (char) toupper(ref[i]);
        i++;
    }
    col_str[i] = '\0';

    *col = col_label_to_index(col_str);
    *row = atoi(ref + i) - 1;
}