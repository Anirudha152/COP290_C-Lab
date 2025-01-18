#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"
#include "compute_unit.h"

int main()
{
    short total_rows = 50;
    short total_cols = 50;

    initStorage(50, 50);
    Value value0;
    value0.type = 0;
    value0.value = 10;

    Value value1;
    value1.type = 1;
    value1.cell = getCell(0, 0);

    setValueExpression(0, 0, value0);
    printf("%d\n", getValue(0, 0));
    setArithmeticExpression(0, 1, value0, value1, 0);

    value0.value = 5;
    setValueExpression(0, 2, value0);

    value1.cell = getCell(0, 1);
    setArithmeticExpression(0, 2, value0, value1, 2);

    value1.cell = getCell(0, 2);
    setArithmeticExpression(0, 0, value0, value1, 2);
    printf("end\n");
    // printf("%d\n", getValue(0, 0));
}
