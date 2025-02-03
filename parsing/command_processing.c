#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "../constants.h"
#include "../backend/compute_unit.h"
#include "../backend/primary_storage.h"
#include "cell_indexing.h"
#include "command_processing.h"


Value parse_value(const char *expr, char **end, short mode) {
    // mode = 0 -> value at the start of function, mode = 1 -> value at the end of function, mode = 2 -> value at the start of arithmetic expression, mode = 3 -> value at the end of arithmetic expression
    Value val = {2, 0, NULL};
    while (*expr == ' ') expr++;

    if (isdigit(*expr) || *expr == '-') {
        val.type = 0;
        val.value = strtol(expr, end, 10);
        if (mode == 1 && **end != ')') {
            val.type = 2;
            return val;
        }
        if (mode == 2 && **end != '+' && **end != '-' && **end != '*' && **end != '/' && **end != '\0') {
            val.type = 2;
            return val;
        }
        if (mode == 3 && **end != '\0') {
            val.type = 2;
            return val;
        }
        return val;
    }

    if (isalpha(*expr)) {
        val.type = 1;
        short row, col;
        const int pos = parse_cell_reference(expr, &row, &col);
        if (!pos) {
            val.type = 2;
            return val;
        }
        if (mode == 1 && *(expr + pos) != ')') {
            val.type = 2;
            return val;
        }
        if (mode == 2 && *(expr + pos) != '+' && *(expr + pos) != '-' && *(expr + pos) != '*' && *(expr + pos) != '/' &&
            *(expr + pos) != '\0') {
            val.type = 2;
            return val;
        }
        if (mode == 3 && *(expr + pos) != '\0') {
            val.type = 2;
            return val;
        }
        val.cell = get_cell(row, col);
        *end = (char *) expr + strcspn(expr, "+-*/(),:) ");
        return val;
    }

    *end = (char *) expr;
    return val;
}

Range parse_range(const char *expr, char **end) {
    Range range = {0, 0, 0, 0, 0};
    char *colon = strchr(expr, ':');
    if (!colon) {
        *end = (char *) expr;
        return range;
    }

    short start_row, start_col, end_row, end_col;

    const int pos1 = parse_cell_reference(expr, &start_row, &start_col);
    if (!pos1) {
        *end = (char *) expr;
        return range;
    }

    if (*(expr + pos1) != ':') {
        *end = (char *) expr;
        return range;
    }

    const int pos2 = parse_cell_reference(colon + 1, &end_row, &end_col);
    if (!pos2) {
        *end = (char *) expr;
        return range;
    }
    if (*(colon + 1 + pos2) != ')') {
        *end = (char *) expr;
        return range;
    }

    if (start_row > end_row || start_col > end_col) {
        *end = (char *) expr;
        return range;
    }

    range.start_row = start_row;
    range.start_col = start_col;
    range.end_row = end_row;
    range.end_col = end_col;
    range.dimension = (start_row == end_row || start_col == end_col) ? 1 : 2;

    *end = strchr(colon, ')');
    return range;
}

char *get_expression_string(const Expression *expr) {
    static char buffer[CMD_BUFFER_SIZE];
    buffer[0] = '\0';
    if (expr->type == 0) {
        if (expr->value1.type == 0) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d", expr->value1.value);
        } else {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d", col_label, expr->value1.cell->row + 1);
        }
    } else if (expr->type == 1) {
        const char op = expr->operation == 0
                            ? '+'
                            : (expr->operation == 1)
                                  ? '-'
                                  : (expr->operation == 2)
                                        ? '*'
                                        : '/';
        if (expr->value1.type == 0 && expr->value2.type == 0) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %d", expr->value1.value, op, expr->value2.value);
        } else if (expr->value1.type == 0 && expr->value2.type == 1) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value2.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %s%d", expr->value1.value, op, col_label,
                     expr->value2.cell->row + 1);
        } else if (expr->value1.type == 1 && expr->value2.type == 0) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %d", col_label, expr->value1.cell->row + 1, op,
                     expr->value2.value);
        } else {
            char col_label[MAX_COL_LABEL];
            char col_label2[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            col_index_to_label(expr->value2.cell->col, col_label2);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %s%d", col_label, expr->value1.cell->row + 1, op, col_label2,
                     expr->value2.cell->row + 1);
        }
    } else if (expr->type == 2) {
        char start_col_label[MAX_COL_LABEL];
        col_index_to_label(expr->function.range.start_col, start_col_label);
        char end_col_label[MAX_COL_LABEL];
        col_index_to_label(expr->function.range.end_col, end_col_label);
        char function_name[6];
        if (expr->function.type == 0) {
            snprintf(function_name, 6, "MIN");
        } else if (expr->function.type == 1) {
            snprintf(function_name, 6, "MAX");
        } else if (expr->function.type == 2) {
            snprintf(function_name, 6, "AVG");
        } else if (expr->function.type == 3) {
            snprintf(function_name, 6, "SUM");
        } else if (expr->function.type == 4) {
            snprintf(function_name, 6, "STDEV");
        } else if (expr->function.type == 5) {
            snprintf(function_name, 6, "SLEEP");
        }
        if (expr->function.type == 5) {
            if (expr->value1.type == 0) {
                snprintf(buffer, CMD_BUFFER_SIZE, "%s(%d)", function_name, expr->value1.value);
            } else {
                col_index_to_label(expr->value1.cell->col, start_col_label);
                snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d)", function_name, start_col_label,
                         expr->value1.cell->row + 1);
            }
        } else {
            snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d:%s%d)", function_name, start_col_label,
                     expr->function.range.start_row + 1, end_col_label, expr->function.range.end_row + 1);
        }
    }

    return buffer;
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

Command process_expression(const char *command, const short viewport_row, const short viewport_col) {
    struct timespec start, finish, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    char *end;
    Value val1;
    Command com;
    char err_message[64];
    strcpy(com.command, command);
    short row, col;
    const int pos = parse_cell_reference(command, &row, &col);
    if (!pos) {
        strcpy(err_message, "Invalid Cell Reference");
        goto error;
    }
    command += (pos + 1);
    // to_upper(command);
    // remove_spaces(command);
    if (strncmp(command, "MIN(", 4) == 0 || strncmp(command, "MAX(", 4) == 0 ||
        strncmp(command, "AVG(", 4) == 0 || strncmp(command, "SUM(", 4) == 0 ||
        strncmp(command, "STDEV(", 6) == 0 || strncmp(command, "SLEEP(", 6) == 0) {
        short func_type = 0;
        if (strncmp(command, "MIN(", 4) == 0) func_type = 0;
        else if (strncmp(command, "MAX(", 4) == 0) func_type = 1;
        else if (strncmp(command, "AVG(", 4) == 0) func_type = 2;
        else if (strncmp(command, "SUM(", 4) == 0) func_type = 3;
        else if (strncmp(command, "STDEV(", 6) == 0) func_type = 4;
        else if (strncmp(command, "SLEEP(", 6) == 0) func_type = 5;

        const char *range_start = strchr(command, '(') + 1;
        if (func_type == 5) {
            val1 = parse_value(range_start, &end, 1);
            if (val1.type == 2) {
                strcpy(err_message, "Invalid Value");
                goto error;
            }
            const int stat = set_sleep_expression(row, col, val1);
            if (!stat) {
                strcpy(err_message, "Circular Dependency");
                goto error;
            }
        } else {
            const Range range = parse_range(range_start, &end);
            if (range.dimension == 0) {
                strcpy(err_message, "Invalid Range");
                goto error;
            }
            const int stat = set_function_expression(row, col, func_type, range);
            if (!stat) {
                strcpy(err_message, "Circular Dependency");
                goto error;
            }
        }
    } else {
        val1 = parse_value(command, &end, 2);
        if (val1.type == 2) {
            strcpy(err_message, "Invalid Value 1");
            goto error;
        }
        if (*end == '+' || *end == '-' || *end == '*' || *end == '/') {
            const char op = *end++;
            const Value val2 = parse_value(end, &end, 3);
            if (val2.type == 2) {
                strcpy(err_message, "Invalid Value 2");
                goto error;
            }
            const short operation = (op == '+') ? 0 : (op == '-') ? 1 : (op == '*') ? 2 : 3;
            const int stat = set_arithmetic_expression(row, col, val1, val2, operation);
            if (!stat) {
                strcpy(err_message, "Circular Dependency");
                goto error;
            }
        } else {
            const int stat = set_value_expression(row, col, val1);
            if (!stat) {
                strcpy(err_message, "Circular Dependency");
                goto error;
            }
        }
    }
    for (short i = viewport_row; i < viewport_row + VIEWPORT_ROWS; i++) {
        for (short j = viewport_col; j < viewport_col + VIEWPORT_ROWS; j++) {
            get_cell_value(i, j);
        }
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    com.time_taken = sub_timespec(start, finish, &delta);
    com.status = 1;
    return com;
error:
    clock_gettime(CLOCK_REALTIME, &finish);
    com.time_taken = sub_timespec(start, finish, &delta);
    com.status = 0;
    strcpy(com.error_msg, err_message);
    return com;
}
