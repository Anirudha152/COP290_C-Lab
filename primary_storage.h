#ifndef primary_storage_h
#define primary_storage_h

#include <stddef.h>
struct Cell;
typedef struct Node
{
    struct Cell *cell;
    struct Node *next;
} Node;
typedef struct
{
    short type;
    int value;
    struct Cell *cell;

} Value;

typedef struct
{
    short dimension;
    struct Cell *cell1;
    struct Cell *cell2;

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

typedef struct Cell
{
    int value;
    short row;
    short col;
    Expression *formula;
    short state;

    struct Cell **dependencies;
    size_t dependency_count;

    Node *head_dependant;
    size_t dependant_count;

} Cell;

void initStorage(short rows, short cols);
void initCell(Cell *cell, short row, short col);
void updateDependencies(short *rows, short *cols, int size, short source_row, short source_col);
int cellValue(short row, short col);
void setValue(short row, short col, int value);
void setFormula(short row, short col, Expression *formula);
void setState(short row, short col, short state);
void addDependant(short source_row, short source_col, short row, short col);
void deleteDependant(short source_row, short source_col, short target_row, short target_col);

#endif