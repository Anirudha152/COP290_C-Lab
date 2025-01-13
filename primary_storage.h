#ifndef primary_storage_h
#define primary_storage_h

#include <stddef.h>

typedef struct
{
    short type;
    int value;
    Cell *cell;

} Value;

typedef struct
{
    short dimension;
    Cell *cell1;
    Cell *cell2;

} Range;

typedef struct
{
    short type;
    Range *range;

} Function;

typedef struct
{

    short type;
    Value *value1;
    Value *value2;
    short operation;
    Function *function;

} Expression;

typedef struct
{
    int value;
    short row;
    short col;
    Expression *formula;
    short state;

    struct Cell *dependencies;
    size_t dependency_count;

    struct Cell *dependants;
    size_t dependant_count;
} Cell;

Cell initCell(Cell *cell, short row, short col);
void updateCell(Cell *cell);
void updateDependencies(Cell *cell);
void updateDependants(Cell *cell);
int cellValue(Cell *cell);

#endif