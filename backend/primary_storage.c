#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"
#include "../data_structures/stack.h"
#include "../data_structures/set.h"

Cell *table;

void initialize_storage() {
    const int size = (int) TOT_ROWS * (int) TOT_COLS;
    table = (Cell *) malloc(sizeof(Cell) * size);

    if (table == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < TOT_ROWS * TOT_COLS; i++) {
        initialize_cell(&table[i], i);
    }
    initialize_stack();
    initialize_stack_mem();
}

void destroy_storage() {
    for (int i = 0; i < TOT_ROWS * TOT_COLS; i++) {
        const Cell *cell = &table[i];
        set_destroy(cell->dependants);
    }
    free(table);
    destroy_stack();
    destroy_stack_mem();
}

void initialize_cell(Cell *cell, const int) {
    cell->value = 0;
    cell->expression_type = 0;
    cell->val1_type = 0;
    cell->val2_type = 0;
    cell->op = 0;
    cell->cell_state = 0;
    cell->val1 = 0;
    cell->val2 = 0;
    cell->dependants = set_create();
}

int get_raw_value(const int cell_index) {
    if (cell_index < 0 || cell_index >= TOT_ROWS * TOT_COLS) {
        printf("Invalid cell reference\n");
        return 0;
    }
    const Cell *cell = &table[cell_index];
    return cell->value;
}

void add_dependant(const int source_cell_index, const int cell_index) {
    if (cell_index < 0 || cell_index >= TOT_ROWS * TOT_COLS || source_cell_index < 0 || source_cell_index >= TOT_ROWS * TOT_COLS) {
        return;
    }
    const Cell *cell = &table[source_cell_index];
    set_insert(cell->dependants, cell_index);
}

void delete_dependant(const int source_cell_index, const int cell_index) {
    const Cell *source = &table[source_cell_index];
    set_remove(source->dependants, cell_index);
}

Cell *get_cell(const int cell_index) {
    return &table[cell_index];
}

Expression get_expression(const int cell_index) {
    if (cell_index < 0 || cell_index >= TOT_ROWS * TOT_COLS) {
        printf("Invalid cell reference\n");
    }
    Cell* cell = get_cell(cell_index);
    Expression exp;
    exp.type = cell->expression_type;
    if (cell->expression_type == 0) {
        Value val;
        if (cell->val1_type == 0) val.type = INTEGER;
        else val.type = CELL_REFERENCE;
        val.value = cell->val1;
        exp.value = val;
    } else if (cell->expression_type == 1) {
        Arithmetic arith;
        arith.type = cell->op;
        Value val1, val2;
        if (cell->val1_type == 0) val1.type = INTEGER;
        else val1.type = CELL_REFERENCE;
        val1.value = cell->val1;
        if (cell->val2_type == 0) val2.type = INTEGER;
        else val2.type = CELL_REFERENCE;
        val2.value = cell->val2;
        arith.value1 = val1;
        arith.value2 = val2;
        exp.arithmetic = arith;
    } else {
        Function func;
        func.type = cell->val2_type * 4 + cell->op;
        if (func.type == 5) {
            Value val;
            if (cell->val1_type == 0) {
                val.type = INTEGER;
            } else {
                val.type = CELL_REFERENCE;
            }
            val.value = cell->val1;
            func.value = val;
        } else {
            int val1 = cell->val1;
            int val2 = cell->val2;
            Range range;
            range.start_index = val1;
            range.end_index = val2;
            func.range = range;
        }
        exp.function = func;
    }
    return exp;
}