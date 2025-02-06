#ifndef primary_storage_h
#define primary_storage_h
#include <stddef.h>
struct Cell;

enum ValueType {
    INTEGER,
    CELL_REFERENCE,
    VALUE_ERROR
};

typedef struct {
    enum ValueType type;
    union {
        int value;
        struct Cell *cell;
    };
} Value;

typedef struct {
    short dimension;
    short start_row;
    short start_col;
    short end_row;
    short end_col;
} Range;

enum FunctionType {
    MIN,
    MAX,
    AVG,
    SUM,
    STDEV,
    SLEEP
};

typedef struct {
    enum FunctionType type;
    union {
        Range range;
        Value value;
    };
} Function;

enum ArithmeticType {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

typedef struct {
    enum ArithmeticType type;
    Value value1;
    Value value2;
} Arithmetic;

enum ExpressionType {
    VALUE,
    ARITHMETIC,
    FUNCTION
};

typedef struct {
    enum ExpressionType type;
    union {
        Value value;
        Arithmetic arithmetic;
        Function function;
    };
} Expression;

typedef struct SetNode {
    struct Cell *cell;
    struct SetNode *left;
    struct SetNode *right;
    struct SetNode *parent;
    int height;
} SetNode;

typedef struct {
    SetNode *root;
    size_t size;
} Set;

typedef struct {
    SetNode *current;
} SetIterator;

enum CellState {
    CLEAN,
    DIRTY,
    DFS_IN_PROGRESS,
    CIRCULAR_CHECKED,
    ZERO_ERROR
};

typedef struct Cell {
    int value;
    short row;
    short col;
    Expression expression;
    enum CellState state;

    struct Cell **dependencies;
    size_t dependency_count;

    Set *dependants;
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
    enum CellState state;
} Memory;
typedef struct {
    int no_of_elements;
    int dynamic_size;
    Memory *elements;
} MemoryStack;

void initialize_storage();

void destroy_storage();

void initialize_cell(Cell *cell, short row, short col);

void update_dependencies(const short *rows, const short *cols, size_t size, short source_row, short source_col);

int get_raw_value(short row, short col);

Cell *get_cell(short row, short col);

void add_dependant(short source_row, short source_col, short row, short col);

void delete_dependant(short source_row, short source_col, short row, short col);

void initialize_expression(Expression *formula);
extern short TOT_ROWS;
extern short TOT_COLS;
#endif
