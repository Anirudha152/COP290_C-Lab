#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"
#include "../data_structures/stack.h"
#include "../data_structures/set.h"

Cell *table;
typedef struct Expression_table{
    Expression* expressions;
    int size;
    int capacity;
} Expression_table;

Expression_table* expression_table;

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
    initialize_expression_table();
    initialize_stack();
    initialize_stack_mem();
}

void initialize_expression_table(){
    expression_table = (Expression_table*)malloc(sizeof(Expression_table));
    if(expression_table == NULL){
        printf("Memory allocation failed\n");
        exit(1);
    }
    expression_table->size = 0;
    expression_table->capacity = 8;
    expression_table->expressions = (Expression*)malloc(sizeof(Expression) * expression_table->capacity);
    if(expression_table->expressions == NULL){
        printf("Memory allocation failed\n");
        exit(1);
    }
}

void destroy_expression_table(){
    free(expression_table->expressions);
    free(expression_table);
}

int expression_index(){
    if(expression_table->size == expression_table->capacity){
        expression_table->capacity *= 2;
        expression_table->expressions = (Expression*)realloc(expression_table->expressions, sizeof(Expression) * expression_table->capacity);
        if(expression_table->expressions == NULL){
            printf("Memory allocation failed\n");
            exit(1);
        }
    }
    return expression_table->size++;
}
void destroy_storage()
{
    for (short i = 0; i < TOT_ROWS; i++)
    {
        for (short j = 0; j < TOT_COLS; j++)
        {
            const Cell *cell = &table[i * TOT_COLS + j];
            set_destroy(cell->dependants);
        }
    }
    free(table);
    destroy_expression_table();
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
    cell->expression_index = -1;
    cell->state = CLEAN;
    cell->dependency_top_left_row = -1;
    cell->dependency_top_left_col = -1;
    cell->dependency_bottom_right_row = -1;
    cell->dependency_bottom_right_col = -1;
    cell->dependency_count = 0;
    cell->dependants = set_create();
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

void update_dependencies(const short *rows, const short *cols, const int size, const short source_row, const short source_col)
{
    Cell *cell = &table[(int)source_row * TOT_COLS + source_col];
    cell->dependency_top_left_row = -1;
    cell->dependency_top_left_col = -1;
    cell->dependency_bottom_right_row = -1;
    cell->dependency_bottom_right_col = -1;
    cell->dependency_count = size;
    if (size == 0)
    {
        return;
    }
    cell->dependency_top_left_row = rows[0];
    cell->dependency_top_left_col = cols[0];
    if (size > 1)
    {
        cell->dependency_bottom_right_row = rows[1];
        cell->dependency_bottom_right_col = cols[1];
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
}

void delete_dependant(const short source_row, const short source_col, const short row, const short col)
{
    Cell *source = &table[(int)source_row * (int)TOT_COLS + (int)source_col];
    set_remove(source->dependants, row, col);
}

Cell *get_cell(const short row, const short col)
{
    return &table[(int)row * (int)TOT_COLS + (int)col];
}

Expression *get_expression(const short row, const short col)
{
    const Cell *cell = get_cell(row, col);
    if (cell->expression_index == -1)
    {
        return NULL;
    }
    return &expression_table->expressions[cell->expression_index];
}

void set_expression_index(short row, short col, Expression expression){
    Cell *cell = get_cell(row, col);
    if(cell->expression_index != -1){
        expression_table->expressions[cell->expression_index] = expression;
    }else{
        const int index = expression_index();
        cell->expression_index = index;
        expression_table->expressions[index] = expression;
    }
}
