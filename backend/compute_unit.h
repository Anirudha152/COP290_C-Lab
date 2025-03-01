#ifndef compute_unit_h
#define compute_unit_h
#include "primary_storage.h"

typedef struct {
	int first;
	int second;
} pair;

int set_value_expression(int cell_index, Value value);

int set_arithmetic_expression(int cell_index, Arithmetic arithmetic);

int set_function_expression(int cell_index, Function function);

int get_cell_value(int cell_index);

extern Stack cell_stack;
#endif
