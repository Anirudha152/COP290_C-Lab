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

char *get_expression_string(const int cell_index) {
    Cell *cell = get_cell(cell_index);
    static char buffer[CMD_BUFFER_SIZE];
    buffer[0] = '\0';
    if (cell->expression_type == VALUE) {
        if (cell->val1_type == INTEGER) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d", cell->val1);
        } else if (cell->val1_type == CELL_REFERENCE) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(cell_index_to_col(cell->val1), col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d", col_label, cell_index_to_row(cell->val1) + 1);
        } else {
            printf("Value holds Error State");
            exit(1);
        }
    } else if (cell->expression_type == ARITHMETIC) {
        Arithmetic arithmetic;
        arithmetic.type = cell->op;
        const Value val1 = {cell->val1, cell->val1_type};
        const Value val2 = {cell->val2, cell->val2_type};
        arithmetic.value1 = val1;
        arithmetic.value2 = val2;
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
            col_index_to_label(cell_index_to_col(arithmetic.value2.value), col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %s%d", arithmetic.value1.value, op, col_label, cell_index_to_row(arithmetic.value2.value) + 1);
        } else if (arithmetic.value1.type == CELL_REFERENCE && arithmetic.value2.type == INTEGER) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(cell_index_to_col(arithmetic.value1.value), col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %d", col_label, cell_index_to_row(arithmetic.value1.value) + 1, op, arithmetic.value2.value);
        } else {
            char col_label[MAX_COL_LABEL];
            char col_label2[MAX_COL_LABEL];
            col_index_to_label(cell_index_to_col(arithmetic.value1.value), col_label);
            col_index_to_label(cell_index_to_col(arithmetic.value2.value), col_label2);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %s%d", col_label, cell_index_to_row(arithmetic.value1.value) + 1, op, col_label2, cell_index_to_row(arithmetic.value2.value) + 1);
        }
    } else if (cell->expression_type == FUNCTION) {
        char function_name[6];
        const int function_type = cell->val2_type * 4 + cell->op;
        if (function_type == MIN) {
            snprintf(function_name, 6, "MIN");
        } else if (function_type == MAX) {
            snprintf(function_name, 6, "MAX");
        } else if (function_type == AVG) {
            snprintf(function_name, 6, "AVG");
        } else if (function_type == SUM) {
            snprintf(function_name, 6, "SUM");
        } else if (function_type == STDEV) {
            snprintf(function_name, 6, "STDEV");
        } else if (function_type == SLEEP) {
            snprintf(function_name, 6, "SLEEP");
        }
        if (function_type == SLEEP) {
            if (cell->val1_type == INTEGER) {
                snprintf(buffer, CMD_BUFFER_SIZE, "%s(%d)", function_name, cell->val1);
            } else {
                char col_label[MAX_COL_LABEL];
                col_index_to_label(cell_index_to_col(cell->val1), col_label);
                snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d)", function_name, col_label,
                         cell_index_to_row(cell->val1) + 1);
            }
        } else {
            char start_col_label[MAX_COL_LABEL];
            col_index_to_label(cell_index_to_col(cell->val1), start_col_label);
            char end_col_label[MAX_COL_LABEL];
            col_index_to_label(cell_index_to_col(cell->val2), end_col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d:%s%d)", function_name, start_col_label,
                     cell_index_to_row(cell->val1) + 1, end_col_label, cell_index_to_row(cell->val2) + 1);
        }
    }

    return buffer;
}

int check_regex(const char* pattern, const char* command) {
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return 0;
    }
    const int regex_result = regexec(&regex, command, 0, NULL, 0);
    regfree(&regex);
    return regex_result == 0;
}

int regex_index(const char* pattern, const char* command) {
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return 0;
    }
    regmatch_t match;
    const int regex_result = regexec(&regex, command, 1, &match, 0);
    regfree(&regex);
    if (regex_result != 0) {
        return 0;
    }
    return match.rm_eo;
}

Command process_expression(const char *command) {
    struct timespec start, finish, delta;
    Command com;
    strcpy(com.command, command);
    char error_msg[64];
    if (GUI) {
        clock_gettime(CLOCK_REALTIME, &start);
    }
    const char regex_main[] = "^[A-Z]{1,3}[1-9][0-9]{0,2}\\=";
    int pos = regex_index(regex_main, command);
    if (!pos) {
        strcpy(error_msg, "Invalid Cell Reference");
        goto error;
    }
    char col_label_1[MAX_COL_LABEL];
    short row;
    sscanf(command, "%[A-Z]%hd=", col_label_1, &row);
    --row;
    const short col = col_label_to_index(col_label_1);
    if (row < 0 || row >= TOT_ROWS || col < 0 || col >= TOT_COLS) {
        strcpy(error_msg, "Cell Reference out of bounds");
        goto error;
    }
    command += pos;
    const char regex_a[] = "^(-?[0-9]+)$";
    const char regex_b[] = "^([A-Z]{1,3}[1-9][0-9]{0,2})$";
    const char regex_c[] = "^(-?[0-9]{1,})[\\+-\\*\\/](-?[0-9]+)$";
    const char regex_d[] = "^(-?[0-9]{1,})[\\+-\\*\\/]([A-Z]{1,3}[1-9][0-9]{0,2})$";
    const char regex_e[] = "^([A-Z]{1,3}[1-9][0-9]{0,2})[\\+-\\*\\/](-?[0-9]+)$";
    const char regex_f[] = "^([A-Z]{1,3}[1-9][0-9]{0,2})[\\+-\\*\\/]([A-Z]{1,3}[1-9][0-9]{0,2})$";
    const char regex_g[] = "^((SUM|MAX|MIN|AVG|STDEV)\\([A-Z]{1,3}[1-9][0-9]{0,2}\\:[A-Z]{1,3}[1-9][0-9]{0,2}\\))$";
    const char regex_h[] = "^(SLEEP\\((-?[0-9]+)\\))$";
    const char regex_i[] = "^(SLEEP\\(([A-Z]{1,3}[1-9][0-9]{0,2})\\))$";
    int stat;
    if (check_regex(regex_a, command)) { // A1=5
        int value;
        sscanf(command, "%d", &value);
        const Value val1 = {.type = INTEGER, .value = value};
        stat = set_value_expression(rowcol_to_cell_index(row, col), val1);
    } else if (check_regex(regex_b, command)) { // A1=B4
        short row_2;
        char col_label_2[MAX_COL_LABEL];
        sscanf(command, "%[A-Z]%hd", col_label_2, &row_2);
        const short col_2 = col_label_to_index(col_label_2);
        --row_2;
        const Value val1 = {.type = CELL_REFERENCE, .value = rowcol_to_cell_index(row_2, col_2)};
        stat = set_value_expression(rowcol_to_cell_index(row, col), val1);
    } else if (check_regex(regex_c, command)) { // A1=4+5
        char op;
        int value1, value2;
        sscanf(command, "%d%c%d", &value1, &op, &value2);
        const Value val1 = {.type = INTEGER, .value = value1};
        const Value val2 = {.type = INTEGER, .value = value2};
        const Arithmetic arithmetic = {.type = op == '+' ? ADD : op == '-' ? SUBTRACT : op == '*' ? MULTIPLY : DIVIDE, .value1 = val1, .value2 = val2};
        stat = set_arithmetic_expression(rowcol_to_cell_index(row, col), arithmetic);
    } else if (check_regex(regex_d, command)) { // A1=4+B5
        char op;
        int value1;
        short row_2;
        char col_label_2[MAX_COL_LABEL];
        sscanf(command, "%d%c%[A-Z]%hd", &value1, &op, col_label_2, &row_2);
        const short col_2 = col_label_to_index(col_label_2);
        --row_2;
        const Value val1 = {.type = INTEGER, .value = value1};
        const Value val2 = {.type = CELL_REFERENCE, .value = rowcol_to_cell_index(row_2, col_2)};
        const Arithmetic arithmetic = {.type = op == '+' ? ADD : op == '-' ? SUBTRACT : op == '*' ? MULTIPLY : DIVIDE, .value1 = val1, .value2 = val2};
        stat = set_arithmetic_expression(rowcol_to_cell_index(row, col), arithmetic);
    } else if (check_regex(regex_e, command)) { // A1=B4+5
        char op;
        short row_2;
        char col_label_2[MAX_COL_LABEL];
        int value2;
        sscanf(command, "%[A-Z]%hd%c%d", col_label_2, &row_2, &op, &value2);
        const short col_2 = col_label_to_index(col_label_2);
        --row_2;
        const Value val1 = {.type = CELL_REFERENCE, .value = rowcol_to_cell_index(row_2, col_2)};
        const Value val2 = {.type = INTEGER, .value = value2};
        const Arithmetic arithmetic = {.type = op == '+' ? ADD : op == '-' ? SUBTRACT : op == '*' ? MULTIPLY : DIVIDE, .value1 = val1, .value2 = val2};
        stat = set_arithmetic_expression(rowcol_to_cell_index(row, col), arithmetic);
    } else if (check_regex(regex_f, command)) { // A1=B4+C5
        char op;
        short row_2, row_3;
        char col_label_2[MAX_COL_LABEL], col_label_3[MAX_COL_LABEL];
        sscanf(command, "%[A-Z]%hd%c%[A-Z]%hd", col_label_2, &row_2, &op, col_label_3, &row_3);
        const short col_2 = col_label_to_index(col_label_2);
        const short col_3 = col_label_to_index(col_label_3);
        --row_2;
        --row_3;
        const Value val1 = {.type = CELL_REFERENCE, .value = rowcol_to_cell_index(row_2, col_2)};
        const Value val2 = {.type = CELL_REFERENCE, .value = rowcol_to_cell_index(row_3, col_3)};
        const Arithmetic arithmetic = {.type = op == '+' ? ADD : op == '-' ? SUBTRACT : op == '*' ? MULTIPLY : DIVIDE, .value1 = val1, .value2 = val2};
        stat = set_arithmetic_expression(rowcol_to_cell_index(row, col), arithmetic);
    } else if (check_regex(regex_g, command)) { // A1=SUM(B4:C5)
        char function_name[6];
        char start_col_label[MAX_COL_LABEL], end_col_label[MAX_COL_LABEL];
        short start_row, end_row;
        sscanf(command, "%[A-Z](%[A-Z]%hd:%[A-Z]%hd)", function_name, start_col_label, &start_row, end_col_label, &end_row);
        const short start_col = col_label_to_index(start_col_label);
        const short end_col = col_label_to_index(end_col_label);
        --start_row;
        --end_row;
        const Range range = {.start_index = rowcol_to_cell_index(start_row, start_col), .end_index = rowcol_to_cell_index(end_row, end_col)};
        Function function;
        if (strcmp(function_name, "MIN") == 0) {
            function.type = MIN;
        } else if (strcmp(function_name, "MAX") == 0) {
            function.type = MAX;
        } else if (strcmp(function_name, "AVG") == 0) {
            function.type = AVG;
        } else if (strcmp(function_name, "SUM") == 0) {
            function.type = SUM;
        } else if (strcmp(function_name, "STDEV") == 0) {
            function.type = STDEV;
        }
        function.range = range;
        stat = set_function_expression(rowcol_to_cell_index(row, col), function);
    } else if (check_regex(regex_h, command)) { // A1=SLEEP(5)
        char function_name[6];
        int value;
        sscanf(command, "%[A-Z](%d)", function_name, &value);
        Function function;
        function.type = SLEEP;
        function.value = (Value){.type = INTEGER, .value = value};
        stat = set_function_expression(rowcol_to_cell_index(row, col), function);
    } else if (check_regex(regex_i, command)) { // A1=SLEEP(B4)
        char function_name[6];
        short row_2;
        char col_label_2[MAX_COL_LABEL];
        sscanf(command, "%[A-Z](%[A-Z]%hd)", function_name, col_label_2, &row_2);
        const short col_2 = col_label_to_index(col_label_2);
        --row_2;
        Function function;
        function.type = SLEEP;
        function.value = (Value){.type = CELL_REFERENCE, .value = rowcol_to_cell_index(row_2, col_2)};
        stat = set_function_expression(rowcol_to_cell_index(row, col), function);
    } else {
        strcpy(error_msg, "Invalid Expression");
        goto error;
    }
    if (stat == 0) {
        strcpy(error_msg, "Circular Dependency");
        goto error;
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
    strcpy(com.error_msg, error_msg);
    return com;
}
