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

    Cell **dependencies;
    size_t dependency_count;

    // Cell **dependants; dependant to be implemented by linked list

} Cell;

void initStorage(short rows, short cols);
void initCell(Cell *cell, short row, short col);
void updateCell(Cell *cell);
void updateDependencies(short *rows, short *cols, short size, Cell *cell);
int cellValue(short row, short col);
// void updateDependencies(Cell *cell);
//  void markDependantsDirty(Cell *cell);
// void addDependant(Cell *cell, Cell *dependant);
void setValue(short row, short col, int value);
void setFormula(short row, short col, Expression *formula);
void setState(short row, short col, short state);
// void deleteDependant(short source_row, short source_col, short target_row, short target_col);

#endif