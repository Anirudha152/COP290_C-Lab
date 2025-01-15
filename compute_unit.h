#ifndef compute_unit_h
#define compute_unit_h

#include "primary_storage.h"

void setValueExpression(short row, short col, short type, int value, short refcell_row, short refcell_col);
void setArithmeticExpression(short row, short col, short operation, short cell1_row, short cell1_col, short cell2_row, short cell2_col);
void setFunctionExpression(short topleft_row, short topleft_col, short bottomright_row, short bottomright_col, short type);
int getValue(short row, short col);



#endif