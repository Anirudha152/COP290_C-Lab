#ifndef compute_unit_h
#define compute_unit_h

#include "primary_storage.h"

void setValueExpression(short row, short col, Value value);
void setArithmeticExpression(short row, short col, Value value1, Value value2, short operation);
void setFunctionExpression(short row, short col, short type, Range range);
int getValue(short row, short col);

#endif