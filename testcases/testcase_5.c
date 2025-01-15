#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"
#include "compute_unit.h"

int main()
{
    short total_rows = 50;
    short total_cols = 50;

    initStorage(50, 50);
    Range range0;
    range0.start_row = 3;
    range0.end_row = 10;
    range0.start_col = 3;
    range0.end_col = 10;
    setFunctionExpression(0, 2, 2, range0);

    // for (int i = 7; i <= 10; i++)
    // {
    //     for (int j = 5; j <= 7; j++)
    //     {
    //         Cell *cell = getCell(i, j);
    //         Node *temp = cell.head_dependant;
    //         while (temp != NULL)
    //         {
    //             printf("%d, %d\n", temp.cell.row, temp.cell.col);
    //             temp = temp.next;
    //         }
    //     }

    // }

    // Cell *temp1 = getCell(0, 2);

    // Cell **temp_dependencies = temp1.dependencies;
    // for (int i = 0; i < temp1.dependency_count; i++)
    // {
    //     printf("%d %d\n", temp_dependencies[i].row, temp_dependencies[i].col);
    // }
    // printf("\n%d\n", temp1.dependency_count);
    Value value1;
    value1.cell = getCell(3, 4);
    value1.type = 1;
    Value value2;
    value2.cell = getCell(4, 6);
    value2.type = 1;
    setValueExpression(0, 0, value1);
    setArithmeticExpression(0, 1, value1, value2, 0);
    Value value3;
    value3.type = 0;
    value3.value = 7;
    setValueExpression(3, 4, value3);
    setValue(3, 4, 5);
    printf("%d, %d, %d\n", getValue(0, 0), getValue(0, 1), getValue(0, 2));
    Value value4;
    value4.type = 0;
    value4.value = 253;
    setValueExpression(4, 6, value4);
    printf("%d, %d, %d\n", getValue(0, 0), getValue(0, 1), getValue(0, 2));
}
