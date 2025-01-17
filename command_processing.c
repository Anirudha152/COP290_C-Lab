#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "constants.h"
#include "compute_unit.h"
#include "primary_storage.h"
#include "cell_indexing.h"
#include "command_processing.h"


Value parse_value(const char *expr, char **end)  {
    Value val = {0, 0, NULL};
    while (*expr == ' ') expr++;

    if (isdigit(*expr) || *expr == '-') {
        val.type = 0;
        val.value = strtol(expr, end, 10);
        return val;
    }

    if (isalpha(*expr)) {
        val.type = 1;
        short row, col;
        parse_cell_reference(expr, &row, &col);

        val.cell = getCell(row, col);

        *end = (char*)expr + strcspn(expr, "+-*/(),:) ");
        return val;
    }

    *end = (char*)expr;
    return val;
}

Range parse_range(const char *expr, char **end) {
    Range range = {0, 0, 0, 0, 0};
    char *colon = strchr(expr, ':');
    if (!colon) {
        *end = (char*)expr;
        return range;
    }

    short start_row, start_col, end_row, end_col;
    parse_cell_reference(expr, &start_row, &start_col);
    parse_cell_reference(colon + 1, &end_row, &end_col);

    if (start_row > end_row || start_col > end_col) {
        *end = (char*)expr;
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

char* get_expression_string(const Expression *expr) {
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
        char op = (expr->operation == 0) ? '+' : (expr->operation == 1) ? '-' : (expr->operation == 2) ? '*' : '/';
        if (expr->value1.type == 0 && expr->value2.type == 0 ) {
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %d", expr->value1.value, op, expr->value2.value);
        } else if (expr->value1.type == 0 && expr->value2.type == 1) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value2.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%d %c %s%d", expr->value1.value, op, col_label, expr->value2.cell->row + 1);
        } else if (expr->value1.type == 1 && expr->value2.type == 0) {
            char col_label[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %d", col_label, expr->value1.cell->row + 1, op, expr->value2.value);
        } else {
            char col_label[MAX_COL_LABEL];
            char col_label2[MAX_COL_LABEL];
            col_index_to_label(expr->value1.cell->col, col_label);
            col_index_to_label(expr->value2.cell->col, col_label2);
            snprintf(buffer, CMD_BUFFER_SIZE, "%s%d %c %s%d", col_label, expr->value1.cell->row + 1, op, col_label2, expr->value2.cell->row + 1);
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
        }
        snprintf(buffer, CMD_BUFFER_SIZE, "%s(%s%d:%s%d)", function_name, start_col_label, expr->function.range.start_row + 1, end_col_label, expr->function.range.end_row + 1);
    }

    return buffer;
}

Command process_expression(char *input, const short row, const short col, const short viewport_row, const short viewport_col) {
    clock_t start = clock();
    char *end;
    Value val1, val2;
    Range range;
    Command com;
    char cmd_str[CMD_BUFFER_SIZE];
    char col_label[MAX_COL_LABEL];
    col_index_to_label(col, col_label);
    snprintf(cmd_str, CMD_BUFFER_SIZE, "%s%d=%s", col_label, row + 1, input);
    strcpy(com.command, cmd_str);

    remove_spaces(input);

    if (strncmp(input, "MIN(", 4) == 0 || strncmp(input, "MAX(", 4) == 0 ||
        strncmp(input, "AVG(", 4) == 0 || strncmp(input, "SUM(", 4) == 0 ||
        strncmp(input, "STDEV(", 6) == 0 || strncmp(input, "SLEEP(", 6) == 0) {

        short func_type = 0;
        if (strncmp(input, "MIN(", 4) == 0) func_type = 0;
        else if (strncmp(input, "MAX(", 4) == 0) func_type = 1;
        else if (strncmp(input, "AVG(", 4) == 0) func_type = 2;
        else if (strncmp(input, "SUM(", 4) == 0) func_type = 3;
        else if (strncmp(input, "STDEV(", 6) == 0) func_type = 4;
        else if (strncmp(input, "SLEEP(", 6) == 0) func_type = 5;

        const char *range_start = strchr(input, '(') + 1;
        if (func_type == 5) {
            val1 = parse_value(range_start, &end);
            setValueExpression(row, col, val1);
        } else {
            range = parse_range(range_start, &end);
            setFunctionExpression(row, col, func_type, range);
        }
    } else {
        val1 = parse_value(input, &end);
        while (*end == ' ') end++;
        if (*end == '+' || *end == '-' || *end == '*' || *end == '/') {
            char op = *end++;
            val2 = parse_value(end, &end);
            short operation = (op == '+') ? 0 : (op == '-') ? 1 : (op == '*') ? 2 : 3;
            setArithmeticExpression(row, col, val1, val2, operation);
        } else {
            setValueExpression(row, col, val1);
        }
    }
    if ((row >= viewport_row && row < viewport_row + VIEWPORT_ROWS && col >= viewport_col && col < viewport_col + VIEWPORT_ROWS) || (viewport_row == -1 && viewport_col == -1)) {
        getValue(row, col);
    }
    com.time_taken = ((double)(clock() - start)) / CLOCKS_PER_SEC;
    com.status = 1;
    return com;
}