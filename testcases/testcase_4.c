#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"
#include "compute_unit.h"

int main()
{
    short total_rows = 50;
    short total_cols = 50;

    initStorage(50, 50);

    Value *value1 = (Value *)malloc(sizeof(Value));
    value1->cell = getCell(3, 4);
    value1->type = 1;
    Value *value2 = (Value *)malloc(sizeof(Value));
    value2->cell = getCell(4, 6);
    value2->type = 1;
    setValueExpression(0, 0, value1);
    setArithmeticExpression(0, 1, value1, value2, 0);
    Value *value3 = (Value *)malloc(sizeof(Value));
    value3->type = 0;
    value3->value = 7;
    // setValueExpression(3,4,value3);
    setValue(3, 4, 5);
    printf("%d, %d\n", getValue(0, 0), getValue(0, 1));
    // Value *value4 = (Value *)malloc(sizeof(Value));
    setValueExpression(4, 6, value3);
    printf("%d, %d\n", getValue(0, 0), getValue(0, 1));
}
