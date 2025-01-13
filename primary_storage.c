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

    table = (Cell *)malloc(sizeof(Cell) * rows * cols);

    if (table == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (short i = 0; i < rows; i++)
    {
        for (short j = 0; j < cols; j++)
        {
            initCell(&table[i * cols + j], i, j);
        }
    }
}

void initCell(Cell *cell, short row, short col)
{
    cell->row = row;
    cell->col = col;
    cell->value = 0;
    cell->formula = NULL;
    cell->state = 0;
    cell->dependencies = NULL;
    cell->dependency_count = 0;
    cell->head_dependant = NULL;
}

int cellValue(short row, short col)
{
    if (row < 0 || row >= total_rows || col < 0 || col >= total_cols)
    {
        // printf("Invalid cell reference\n");
        return 0;
    }

    Cell *cell = &table[row * total_cols + col];
    // if (cell->state == 2)
    // {
    //     updateCell(cell);
    // }
    return cell->value;
}

// void updateDependencies(Cell *cell, Cell **dependency)
// {
//     cell->dependencies = dependency;
// }

// void updateDependants(Cell *cell, Cell **dependant)
// {
//     cell->dependants = dependant;
// }

void updateDependencies(short *rows, short *cols, short size, Cell *cell)
{
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

    for (short i = 0; i < size; i++)
    {
        if (rows[i] < 0 || rows[i] >= total_rows || cols[i] < 0 || cols[i] >= total_cols)
        {
            printf("Invalid cell reference\n");
            exit(1);
        }
        dependency[i] = &table[rows[i] * total_cols + cols[i]];
    }

    cell->dependencies = dependency;
}

// implementation of dependants remaining
void addDependant(Cell *cell, short row, short col)
{
    if (row < 0 || row >= total_rows || col < 0 || col >= total_cols)
    {
        printf("Invalid cell reference\n");
        exit(1);
    }

    Cell *dependant = &table[row * total_cols + col];
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
    Cell *cell = &table[row * total_cols + col];
    cell->value = value;
}

void setFormula(short row, short col, Expression *formula)
{
    Cell *cell = &table[row * total_cols + col];
    cell->formula = formula;
}

void setState(short row, short col, short state)
{
    Cell *cell = &table[row * total_cols + col];
    cell->state = state;
}

void deleteDependants(short sorce_row, short source_col, short target_row, short target_col)
{
    Cell *source = &table[sorce_row * total_cols + source_col];
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
}