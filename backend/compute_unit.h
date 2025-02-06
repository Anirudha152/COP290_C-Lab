#ifndef compute_unit_h
#define compute_unit_h
#include "primary_storage.h"
typedef struct {
	int first;
	int second;
} pair;
int set_value_expression(short row, short col, Value value);
int set_arithmetic_expression(short row, short col, Arithmetic arithmetic);
int set_function_expression(short row, short col, Function function);
int get_cell_value(short row, short col);
extern Stack cell_stack;
#endif