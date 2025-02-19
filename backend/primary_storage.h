#ifndef primary_storage_h
#define primary_storage_h
#include <stddef.h>
struct Cell;

enum ValueType
{
    INTEGER,
    CELL_REFERENCE,
    VALUE_ERROR
};

typedef struct
{
    union
    {
        int value;
        struct Cell *cell;
    };
    enum ValueType type : 2;
} __attribute__((packed)) Value;

typedef struct
{
    short dimension;
    short start_row;
    short start_col;
    short end_row;
    short end_col;
} __attribute__((packed)) Range;

enum FunctionType
{
    MIN,
    MAX,
    AVG,
    SUM,
    STDEV,
    SLEEP
};

typedef struct
{
    enum FunctionType type : 3;
    union
    {
        Range range;
        Value value;
    };
} __attribute__((packed)) Function;

enum ArithmeticType
{
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

typedef struct
{
    enum ArithmeticType type : 2;
    Value value1;
    Value value2;
} __attribute__((packed)) Arithmetic;

enum ExpressionType
{
    VALUE,
    ARITHMETIC,
    FUNCTION
};

typedef struct
{
    enum ExpressionType type : 2;
    union
    {
        Value value;
        Arithmetic arithmetic;
        Function function;
    };
} __attribute__((packed)) Expression;

typedef struct SetNode
{
    struct Cell *cell;
    struct SetNode *left;
    struct SetNode *right;
    struct SetNode *parent;
    int height : 8;
} __attribute__((packed)) SetNode;

typedef struct
{
    SetNode *root;
    int size;
} __attribute__((packed)) Set;

typedef struct
{
    SetNode *current;
} SetIterator;

enum CellState
{
    CLEAN,
    DIRTY,
    DFS_IN_PROGRESS,
    CIRCULAR_CHECKED,
    ZERO_ERROR
};

typedef struct Cell
{
    int value;
    short row;
    short col;
    int expression_index;
    enum CellState state : 3;
    short dependency_top_left_row;
    short dependency_top_left_col;
    short dependency_bottom_right_row;
    short dependency_bottom_right_col;

    int dependency_count;

    Set *dependants;
} __attribute__((packed)) Cell;

typedef struct
{
    int no_of_elements;
    int dynamic_size;
    Cell **elements;
} Stack;

typedef struct
{
    short row;
    short col;
    enum CellState state : 3;
} Memory;
typedef struct
{
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

Expression *get_expression(short row, short col);

void set_expression_index(short row, short col, Expression expression);

void delete_dependant(short source_row, short source_col, short row, short col);

void initialize_expression(Expression *formula);
extern short TOT_ROWS;
extern short TOT_COLS;
#endif
