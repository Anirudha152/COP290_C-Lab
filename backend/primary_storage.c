#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"
#include "../data_structures/stack.h"
#include "../data_structures/set.h"

Cell *table;

void initialize_storage()
{
    const int size = (int)TOT_ROWS * (int)TOT_COLS;
    table = (Cell *)malloc(sizeof(Cell) * size);

    if (table == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (short i = 0; i < TOT_ROWS; i++)
    {
        for (short j = 0; j < TOT_COLS; j++)
        {
            initialize_cell(&table[i * TOT_COLS + j], i, j);
        }
    }
    initialize_stack();
    initialize_stack_mem();
}

void destroy_storage()
{
    for (short i = 0; i < TOT_ROWS; i++)
    {
        for (short j = 0; j < TOT_COLS; j++)
        {
            const Cell *cell = &table[i * TOT_COLS + j];
            if (cell->dependency_top_left != NULL)
            {
                free(cell->dependency_top_left);
            }
            if (cell->dependency_bottom_right != NULL)
            {
                free(cell->dependency_bottom_right);
            }
            set_destroy(cell->dependants);
        }
    }
    free(table);
    destroy_stack();
    destroy_stack_mem();
}

void initialize_expression(Expression *expression)
{
    expression->type = VALUE;
    Value value;
    value.type = INTEGER;
    value.value = 0;
    expression->value = value;
}

void initialize_cell(Cell *cell, const short row, const short col)
{
    cell->row = row;
    cell->col = col;
    cell->value = 0;
    Expression expression;
    initialize_expression(&expression);
    cell->expression = expression;
    cell->state = CLEAN;
    cell->dependency_top_left = NULL;
    cell->dependency_bottom_right = NULL;
    cell->dependency_count = 0;
    cell->dependants = set_create();
    cell->dependant_count = 0;
}

int get_raw_value(const short row, const short col)
{
    if (row < 0 || row >= TOT_ROWS || col < 0 || col >= TOT_COLS)
    {
        printf("Invalid cell reference\n");
        return 0;
    }
    const Cell *cell = &table[(int)row * (int)TOT_COLS + (int)col];
    return cell->value;
}

void update_dependencies(const short *rows, const short *cols, const size_t size, const short source_row, const short source_col)
{
    Cell *cell = &table[(int)source_row * TOT_COLS + source_col];
    cell->dependency_top_left = NULL;
    cell->dependency_bottom_right = NULL;
    cell->dependency_count = size;
    if (cell->dependency_top_left == NULL || cell->dependency_bottom_right == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    if (size == 0)
    {
        return;
    }
    cell->dependency_top_left = &table[(int)rows[0] * TOT_COLS + cols[0]];
    if (size > 1)
    {
        cell->dependency_bottom_right = &table[(int)rows[1] * TOT_COLS + cols[1]];
    }
    cell->dependency_count = size;
    // Cell **dependency = malloc(sizeof(Cell *) * size);
    // if (dependency == NULL) {
    //     printf("Memory allocation failed\n");
    //     exit(1);
    // }
    // for (int i = 0; i < size; i++) {
    //     if (rows[i] < 0 || rows[i] >= TOT_ROWS || cols[i] < 0 || cols[i] >= TOT_COLS) {
    //         printf("Invalid cell reference\n");
    //         exit(1);
    //     }
    //     dependency[i] = &table[(int) rows[i] * (int) TOT_COLS + (int) cols[i]];
    // }
    // cell->dependencies = dependency;
    // cell->dependency_count = size;
}

void add_dependant(const short source_row, const short source_col, const short row, const short col)
{
    if (row < 0 || row >= TOT_ROWS || col < 0 || col >= TOT_COLS)
    {
        return;
    }
    Cell *cell = &table[(int)source_row * (int)TOT_COLS + (int)source_col];
    Cell *dependant = &table[(int)row * (int)TOT_COLS + (int)col];
    set_insert(cell->dependants, dependant);
    cell->dependant_count = set_size(cell->dependants);
}

void delete_dependant(const short source_row, const short source_col, const short row, const short col)
{
    Cell *source = &table[(int)source_row * (int)TOT_COLS + (int)source_col];
    set_remove(source->dependants, row, col);
    source->dependant_count = set_size(source->dependants);
}

Cell *get_cell(const short row, const short col)
{
    return &table[(int)row * (int)TOT_COLS + (int)col];
}
