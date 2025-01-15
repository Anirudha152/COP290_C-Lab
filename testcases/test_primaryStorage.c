#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"

int main()
{
    short rows = 5;
    short cols = 5;

    initStorage(rows, cols);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            setValue(i, j, i + j);
        }
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%d ", cellValue(i, j));
        }
        printf("\n");
    }

    return 0;
}