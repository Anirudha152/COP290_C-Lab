#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"

short total_rows;
short total_cols;
Cell *table;

void initStorage(short rows, short cols)
{
    total_rows = rows;
    total_cols = cols;

    int size = (int)rows * (int)cols;
    table = (Cell *)malloc(sizeof(Cell) * size);

    if (table == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            initCell(&table[i * cols + j], i, j);
        }
    }
}

void initExpression(Expression *formula)
{
    formula->type = 0;
    Value value;
    value.type = 0;
    value.value = 0;
    formula->value1 = value;
    
    
    formula->value2 = value;
    formula->operation = 0;
    Function function;
    formula->function = function;
}

void initCell(Cell *cell, short row, short col)
{
    cell->row = row;
    cell->col = col;
    cell->value = 0;
    Expression formula;
    initExpression(&formula);
    cell->formula = formula;
    cell->state = 0;
    cell->dependencies = NULL;
    cell->dependency_count = 0;
    cell->head_dependant = NULL;
}

int cellValue(short row, short col)
{
    if (row < 0 || row >= total_rows || col < 0 || col >= total_cols)
    {
        printf("Invalid cell reference\n");
        return 0;
    }

    Cell *cell = &table[(int)row * (int)total_cols + (int)col];
    // if (cell->state == 2)
    // {
    //     updateCell(cell);
    // }
    return cell->value;
}

void updateDependencies(short *rows, short *cols, int size, short source_row, short source_col)
{
    Cell *cell = &table[(int)source_row * total_cols + source_col];
    if (cell->dependencies != NULL)
    {
        free(cell->dependencies);
        cell->dependencies = NULL;
    }

    Cell **dependency = (Cell **)malloc(sizeof(Cell *) * size);
    if (dependency == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < size; i++)
    {
        if (rows[i] < 0 || rows[i] >= total_rows || cols[i] < 0 || cols[i] >= total_cols)
        {
            printf("Invalid cell reference\n");
            exit(1);
        }
        dependency[i] = &table[(int)rows[i] * (int)total_cols + (int)cols[i]];
    }

    cell->dependencies = dependency;
    cell->dependency_count = size;
}

void addDependant(short source_row, short source_col, short row, short col)
{
    if (row < 0 || row >= total_rows || col < 0 || col >= total_cols)
    {
        printf("Invalid cell reference\n");
        exit(1);
    }

    Cell *cell = &table[(int)source_row * (int)total_cols + (int)source_col];

    Cell *dependant = &table[(int)row * (int)total_cols + (int)col];
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    new_node->cell = dependant;

    // check if dependant is already present
    Node *temp = cell->head_dependant;
    while (temp != NULL)
    {
        if (temp->cell == dependant)
        {
            return;
        }
        temp = temp->next;
    }

    new_node->next = cell->head_dependant;
    cell->head_dependant = new_node;

    cell->dependant_count++;
}

void setValue(short row, short col, int value)
{
    Cell *cell = &table[(int)row * (int)total_cols + (int)col];
    cell->value = value;
}


void setState(short row, short col, short state)
{
    Cell *cell = &table[(int)row * (int)total_cols + (int)col];
    cell->state = state;
}

void deleteDependant(short sorce_row, short source_col, short target_row, short target_col)
{
    Cell *source = &table[(int)sorce_row * (int)total_cols + (int)source_col];
    Node *temp = source->head_dependant;
    Node *prev = NULL;

    while (temp != NULL)
    {
        if (temp->cell->row == target_row && temp->cell->col == target_col)
        {
            if (prev == NULL)
            {
                source->head_dependant = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    source->dependant_count--;
    
}
Cell *getCell(short row, short col)
{
    return &table[(int)row * (int)total_cols + (int)col];
}
