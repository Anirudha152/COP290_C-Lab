#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <regex.h>
#include "../constants.h"
#include "../backend/compute_unit.h"
#include "../backend/primary_storage.h"
#include "cell_indexing.h"
#include "command_processing.h"


Value parse_value(const char *expression_str, char **end, short mode) {
    // mode = 0 -> value at the start of function, mode = 1 -> value at the end of function, mode = 2 -> value at the start of arithmetic expression, mode = 3 -> value at the end of arithmetic expression
    Value val = {VALUE_ERROR, 0};

    if (isdigit(*expression_str) || (*expression_str == '-' && isdigit(*(expression_str + 1)))) {
        val.type = INTEGER;
        val.value = strtol(expression_str, end, 10);
        if (mode == 1 && **end != ')') {
            val.type = VALUE_ERROR;
            return val;
        }
        if (mode == 2 && **end != '+' && **end != '-' && **end != '*' && **end != '/' && **end != '\0') {
            val.type = VALUE_ERROR;
            return val;
        }
        if (mode == 3 && **end != '\0') {
            val.type = VALUE_ERROR;
            return val;
        }
        return val;
    }

    if (isalpha(*expression_str)) {
        val.type = CELL_REFERENCE;
        short row, col;
        const int pos = parse_cell_reference(expression_str, &row, &col);
        if (!pos) {
            val.type = VALUE_ERROR;
            return val;
        }
        if (mode == 1 && *(expression_str + pos) != ')') {
            val.type = VALUE_ERROR;
            return val;
        }
        if (mode == 2 && *(expression_str + pos) != '+' && *(expression_str + pos) != '-' && *(expression_str + pos) != '*' && *(expression_str + pos) != '/' &&
            *(expression_str + pos) != '\0') {
            val.type = VALUE_ERROR;
            return val;
        }
        if (mode == 3 && *(expression_str + pos) != '\0') {
            val.type = VALUE_ERROR;
            return val;
        }
        val.cell = get_cell(row, col);
        *end = (char *) expression_str + strcspn(expression_str, "+-*/(),:) ");
        return val;
    }

    *end = (char *) expression_str;
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
    range.dimension = start_row == end_row || start_col == end_col ? 1 : 2;
    *end = strchr(colon, ')');
    return range;
}

char *get_expression_string(const Expression expression) {
    static char buffer[CMD_BUFFER_SIZE];
    buffer[0] = '\0';
    if (expression.type == VALUE) {
        if (expression.value.type == INTEGER) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d", expression.value.value);
        } else if (expression.value.type == CELL_REFERENCE) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expression.value.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d", col_label, expression.value.cell->row + 1);
        } else {
            printf("Value holds Error State");
            exit(1);
        }
    } else if (expression.type == ARITHMETIC) {
        const Arithmetic arithmetic = expression.arithmetic;
        const char op = arithmetic.type == ADD
                            ? '+'
                            : arithmetic.type == SUBTRACT
                                  ? '-'
                                  : arithmetic.type == MULTIPLY
                                        ? '*'
                                        : '/';
        if (arithmetic.value1.type == INTEGER && arithmetic.value2.type == INTEGER) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %d", arithmetic.value1.value, op, arithmetic.value2.value);
        } else if (arithmetic.value1.type == INTEGER && arithmetic.value2.type == CELL_REFERENCE) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(arithmetic.value2.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %s%d", arithmetic.value1.value, op, col_label, arithmetic.value2.cell->row + 1);
        } else if (arithmetic.value1.type == CELL_REFERENCE && arithmetic.value2.type == INTEGER) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(arithmetic.value1.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %d", col_label, arithmetic.value1.cell->row + 1, op, arithmetic.value2.value);
        } else {
            char col_label[MAX_COL_LABEL];
            char col_label2[MAX_COL_LABEL];
            col_index_to_label(arithmetic.value1.cell->col, col_label);
            col_index_to_label(arithmetic.value2.cell->col, col_label2);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %s%d", col_label, arithmetic.value1.cell->row + 1, op, col_label2, arithmetic.value2.cell->row + 1);
        }
    } else if (expression.type == FUNCTION) {
        char function_name[6];
        if (expression.function.type == MIN) {
            snprintf(function_name, 6, "MIN");
        } else if (expression.function.type == MAX) {
            snprintf(function_name, 6, "MAX");
        } else if (expression.function.type == AVG) {
            snprintf(function_name, 6, "AVG");
        } else if (expression.function.type == SUM) {
            snprintf(function_name, 6, "SUM");
        } else if (expression.function.type == STDEV) {
            snprintf(function_name, 6, "STDEV");
        } else if (expression.function.type == SLEEP) {
            snprintf(function_name, 6, "SLEEP");
        }
        if (expression.function.type == SLEEP) {
            if (expression.function.value.type == INTEGER) {
                snprintf(buffer, CMD_BUFFER_SIZE, "%s(%d)", function_name, expression.function.value.value);
            } else {
                char col_label[MAX_COL_LABEL];
                col_index_to_label(expression.function.value.cell->col, col_label);
                snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d)", function_name, col_label,
                         expression.function.value.cell->row + 1);
            }
        } else {
            char start_col_label[MAX_COL_LABEL];
            col_index_to_label(expression.function.range.start_col, start_col_label);
            char end_col_label[MAX_COL_LABEL];
            col_index_to_label(expression.function.range.end_col, end_col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d:%s%d)", function_name, start_col_label,
                     expression.function.range.start_row + 1, end_col_label, expression.function.range.end_row + 1);
        }
    }

    return buffer;
}

Command process_expression(const char *command, const short viewport_row, const short viewport_col) {
    struct timespec start, finish, delta;
    Command com;
    strcpy(com.command, command);

    if (GUI) {
        clock_gettime(CLOCK_REALTIME, &start);
    }

    regex_t regex;
    int regex_result;
    char regex_pattern[] = "^[A-Z]{1,3}[1-9]{1,1}[0-9]{0,2}\\=((SUM|MAX|MIN|AVG|STDEV)\\([A-Z]{1,3}[1-9]{1,1}[0-9]{0,2}\\:[A-Z]{1,3}[1-9]{1,1}[0-9]{0,2}\\)|SLEEP\\((-?[0-9]{1,}|[A-Z]{1,3}[1-9]{1,1}[0-9]{0,2})\\)|(-?[0-9]{1,}|[A-Z]{1,3}[1-9]{1,1}[0-9]{0,2})[\\+\\-\\*\\/](-?[0-9]{1,}|[A-Z]{1,3}[1-9]{1,1}[0-9]{0,2})|(-?[0-9]{1,}|[A-Z]{1,3}[1-9]{1,1}[0-9]{0,2}))$";

    if (regcomp(&regex, regex_pattern, REG_EXTENDED) != 0) {
        strcpy(com.error_msg, "Internal Regex Compilation Error");
        com.status = 0;
        if (GUI) {
            clock_gettime(CLOCK_REALTIME, &finish);
            com.time_taken = sub_timespec(start, finish, &delta);
        }
        return com;
    }

    regex_result = regexec(&regex, command, 0, NULL, 0);
    regfree(&regex);
    if (regex_result != 0) {
        strcpy(com.error_msg, "Parsing Error");
        com.status = 0;
        if (GUI) {
            clock_gettime(CLOCK_REALTIME, &finish);
            com.time_taken = sub_timespec(start, finish, &delta);
        }
        return com;
    }
    char *end;
    Value val1;
    char err_message[64];
    short row, col;
    const int pos = parse_cell_reference(command, &row, &col);
    if (!pos) {
        strcpy(err_message, "Invalid Command");
        goto error;
    }
    command += pos + 1;
    // to_upper(command);
    // remove_spaces(command);
    if (strncmp(command, "MIN(", 4) == 0 || strncmp(command, "MAX(", 4) == 0 ||
        strncmp(command, "AVG(", 4) == 0 || strncmp(command, "SUM(", 4) == 0 ||
        strncmp(command, "STDEV(", 6) == 0 || strncmp(command, "SLEEP(", 6) == 0) {
        enum FunctionType func_type = MIN;
        if (strncmp(command, "MIN(", 4) == 0) func_type = MIN;
        else if (strncmp(command, "MAX(", 4) == 0) func_type = MAX;
        else if (strncmp(command, "AVG(", 4) == 0) func_type = AVG;
        else if (strncmp(command, "SUM(", 4) == 0) func_type = SUM;
        else if (strncmp(command, "STDEV(", 6) == 0) func_type = STDEV;
        else if (strncmp(command, "SLEEP(", 6) == 0) func_type = SLEEP;
        Function function;
        function.type = func_type;
        const char *range_start = strchr(command, '(') + 1;
        if (func_type == SLEEP) {
            val1 = parse_value(range_start, &end, 1);
            if (val1.type == VALUE_ERROR) {
                strcpy(err_message, "Invalid Value");
                goto error;
            }
            function.value = val1;
        } else {
            const Range range = parse_range(range_start, &end);
            if (range.dimension == 0) {
                strcpy(err_message, "Invalid Range");
                goto error;
            }
            function.range = range;
        }
        const int stat = set_function_expression(row, col, function);
        if (!stat) {
            strcpy(err_message, "Circular Dependency");
            goto error;
        }
    } else {
        val1 = parse_value(command, &end, 2);
        if (val1.type == VALUE_ERROR) {
            strcpy(err_message, "Invalid Value 1");
            goto error;
        }
        if (*end == '+' || *end == '-' || *end == '*' || *end == '/') {
            const char op = *end++;
            const Value val2 = parse_value(end, &end, 3);
            if (val2.type == VALUE_ERROR) {
                strcpy(err_message, "Invalid Value 2");
                goto error;
            }
            Arithmetic arithmetic;
            arithmetic.value1 = val1;
            arithmetic.value2 = val2;
            arithmetic.type = (op == '+') ? ADD : (op == '-') ? SUBTRACT : (op == '*') ? MULTIPLY : DIVIDE;
            const int stat = set_arithmetic_expression(row, col, arithmetic);
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
        if (i >= TOT_ROWS) break;
        for (short j = viewport_col; j < viewport_col + VIEWPORT_ROWS; j++) {
            if (j >= TOT_COLS) break;
            get_cell_value(i, j);
        }
    }
    if (GUI) {
        clock_gettime(CLOCK_REALTIME, &finish);
        com.time_taken = sub_timespec(start, finish, &delta);
    }
    com.status = 1;
    return com;
error:
    if (GUI) {
        clock_gettime(CLOCK_REALTIME, &finish);
        com.time_taken = sub_timespec(start, finish, &delta);
    }
    com.status = 0;
    strcpy(com.error_msg, err_message);
    return com;
}
