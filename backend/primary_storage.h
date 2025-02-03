#ifndef primary_storage_h
#define primary_storage_h

#include <stddef.h>
struct Cell;

typedef struct Node {
    struct Cell *cell;
    struct Node *next;
} Node;

typedef struct {
    short type;
    int value;
    struct Cell *cell;
} Value;

typedef struct {
    short dimension;
    short start_row;
    short start_col;
    short end_row;
    short end_col;
} Range;

typedef struct {
    short type;
    Range range;
} Function;

typedef struct {
    short type;
    Value value1;
    Value value2;
    short operation;
    Function function;
} Expression;

typedef struct Cell {
    int value;
    short row;
    short col;
    Expression formula;
    short state;

    struct Cell **dependencies;
    size_t dependency_count;

    Node *head_dependant;
    size_t dependant_count;
} Cell;

typedef struct {
    int no_of_elements;
    int dynamic_size;
    Cell **elements;
} Stack;

typedef struct {
    short row;
    short col;
    short state;
} Memory;
typedef struct {
    int no_of_elements;
    int dynamic_size;
    Memory *elements;
} MemoryStack;

void initialize_storage();

void destroy_storage();

void initialize_cell(Cell *cell, short row, short col);

void update_dependencies(const short *rows, const short *cols, int size, short source_row, short source_col);

int get_raw_value(short row, short col);

Cell *get_cell(short row, short col);

void add_dependant(short source_row, short source_col, short row, short col);

void delete_dependant(short source_row, short source_col, short target_row, short target_col);

void initialize_expression(Expression *formula);

extern short tot_rows;
extern short tot_cols;

#endif
