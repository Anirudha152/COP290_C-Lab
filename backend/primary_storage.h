#ifndef primary_storage_h
#define primary_storage_h
struct Cell;

enum CellState {
    CLEAN,
    DFS_IN_PROGRESS,
    CIRCULAR_CHECKED,
    ZERO_ERROR
};

enum DependantsType {
    ArrayForm,
    SetForm
};

typedef struct DependantsArray{
    int dependants_cells[4];
    int size;
} __attribute__((packed)) DependantsArray;

typedef struct SetNode {
    int cell_index;
    struct SetNode *left;
    struct SetNode *right;
    struct SetNode *parent;
    int height: 8;
} __attribute__((packed)) SetNode;

typedef struct {
    SetNode *root;
    int size;
} __attribute__((packed)) Set;

typedef struct {
    SetNode *current;
} SetIterator;

enum ValueType {
    INTEGER,
    CELL_REFERENCE,
    VALUE_ERROR
};

typedef struct {
    int value;
    enum ValueType type: 2;
} __attribute__((packed)) Value;

typedef struct {
    int start_index;
    int end_index;
} __attribute__((packed)) Range;

enum FunctionType {
    MIN,
    MAX,
    AVG,
    SUM,
    STDEV,
    SLEEP
};

typedef struct {
    enum FunctionType type: 3;
    union {
        Range range;
        Value value;
    };
} __attribute__((packed)) Function;

enum ArithmeticType {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

typedef struct {
    enum ArithmeticType type: 2;
    Value value1;
    Value value2;
} __attribute__((packed)) Arithmetic;

enum ExpressionType {
    VALUE,
    ARITHMETIC,
    FUNCTION
};

typedef struct {
    enum ExpressionType type: 2;
    union {
        Value value;
        Arithmetic arithmetic;
        Function function;
    };
} __attribute__((packed)) Expression;

typedef struct Cell {
    int value;
    unsigned int expression_type: 2;
    unsigned int val1_type: 1;
    unsigned int val2_type: 1;
    unsigned int op: 2;
    unsigned int cell_state: 2;
    int val1;
    int val2;
    enum DependantsType dependants_type: 2;
    union {
        DependantsArray* dependants_array;
        Set* dependants_set;
    };
} __attribute__((packed)) Cell;

typedef struct {
    int no_of_elements;
    int dynamic_size;
    int *elements;
} Stack;

typedef struct {
    int cell_index;
    enum CellState state: 2;
} Memory;

typedef struct {
    int no_of_elements;
    int dynamic_size;
    Memory *elements;
} MemoryStack;

void initialize_storage();

void destroy_storage();

void initialize_cell(Cell *cell, int cell_index);

int get_raw_value(int cell_index);

Cell *get_cell(int cell_index);

void add_dependant(int source_cell_index, int cell_index);

void delete_dependant(int source_cell_index, int cell_index);

Expression get_expression(int cell_index);

extern short TOT_ROWS;
extern short TOT_COLS;
#endif
