#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"
#include "../data_structures/stack.h"
#include "../data_structures/set.h"

Cell *table;

void initialize_storage() {
    const int size = (int) tot_rows * (int) tot_cols;
    table = (Cell *) malloc(sizeof(Cell) * size);

    if (table == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (short i = 0; i < tot_rows; i++) {
        for (short j = 0; j < tot_cols; j++) {
            initialize_cell(&table[i * tot_cols + j], i, j);
        }
    }
    initialize_stack();
    initialize_stack_mem();
}

void destroy_storage() {
    for (short i = 0; i < tot_rows; i++) {
        for (short j = 0; j < tot_cols; j++) {
            const Cell *cell = &table[i * tot_cols + j];
            if (cell->dependencies != NULL) {
                free(cell->dependencies);
            }
            set_destroy(cell->dependants);
        }
    }
    free(table);
    destroy_stack();
    destroy_stack_mem();
}

void initialize_expression(Expression *formula) {
    formula->type = 0;
    Value value;
    value.type = 0;
    value.value = 0;
    value.cell = NULL;
    formula->value1 = value;


    formula->value2 = value;
    formula->operation = 0;
    Function function;
    function.type = 0;
    Range range;
    range.start_row = 0;
    range.start_col = 0;
    range.end_row = 0;
    range.end_col = 0;
    range.dimension = 0;
    function.range = range;
    formula->function = function;
}

void initialize_cell(Cell *cell, const short row, const short col) {
    cell->row = row;
    cell->col = col;
    cell->value = 0;
    Expression formula;
    initialize_expression(&formula);
    cell->formula = formula;
    cell->state = 0;
    cell->dependencies = NULL;
    cell->dependency_count = 0;
    cell->dependants = set_create();
    cell->dependant_count = 0;
}

int get_raw_value(const short row, const short col) {
    if (row < 0 || row >= tot_rows || col < 0 || col >= tot_cols) {
        printf("Invalid cell reference\n");
        return 0;
    }

    const Cell *cell = &table[(int) row * (int) tot_cols + (int) col];
    return cell->value;
}

void update_dependencies(const short *rows, const short *cols, const size_t size, const short source_row, const short source_col) {
    Cell *cell = &table[(int) source_row * tot_cols + source_col];
    if (cell->dependencies != NULL) {
        free(cell->dependencies);
        cell->dependencies = NULL;
    }

    Cell **dependency = malloc(sizeof(Cell *) * size);
    if (dependency == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < size; i++) {
        if (rows[i] < 0 || rows[i] >= tot_rows || cols[i] < 0 || cols[i] >= tot_cols) {
            printf("Invalid cell reference\n");
            exit(1);
        }
        dependency[i] = &table[(int) rows[i] * (int) tot_cols + (int) cols[i]];
    }

    cell->dependencies = dependency;
    cell->dependency_count = size;
}

void add_dependant(const short source_row, const short source_col, const short row, const short col) {
    if (row < 0 || row >= tot_rows || col < 0 || col >= tot_cols) {
        return;
    }

    Cell *cell = &table[(int) source_row * (int) tot_cols + (int) source_col];

    Cell *dependant = &table[(int) row * (int) tot_cols + (int) col];
    set_insert(cell->dependants, dependant);
    cell->dependant_count = set_size(cell->dependants);
}

void delete_dependant(const short source_row, const short source_col, const short row, const short col) {
    Cell *source = &table[(int) source_row * (int) tot_cols + (int) source_col];
    set_remove(source->dependants, row, col);
    source->dependant_count = set_size(source->dependants);
}

Cell *get_cell(const short row, const short col) {
    return &table[(int) row * (int) tot_cols + (int) col];
}
