#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"

int main()
{
    short total_rows = 15;
    short total_cols = 15;

    initStorage(total_rows, total_cols);
    short *rows1 = (short *)malloc(sizeof(short) * 5);
    short *cols1 = (short *)malloc(sizeof(short) * 5);
    int size = 5;

    for (int i = 2; i < 7; i++)
    {
        rows1[i - 2] = i;
        cols1[i - 2] = 2;
    }

    updateDependencies(rows1, cols1, size, 0, 0);

    Cell *cell = getCell(0, 0);

    // if (cell->dependency_count != 5)
    // {
    //     printf("Test failed, here\n");
    //     return 1;
    // }

    for (int i = 0; i < 5; i++)
    {
        if (cell->dependencies[i]->row != rows1[i] || cell->dependencies[i]->col != cols1[i])
        {

            printf("Test failed, here2\n");
            return 1;
        }
        else
        {
            printf("%d %d\n", cell->dependencies[i]->row, cell->dependencies[i]->col);
        }
    }

    printf("Test passed\n");

    short *rows2 = (short *)malloc(sizeof(short) * 4);
    short *cols2 = (short *)malloc(sizeof(short) * 4);
    size = 4;

    for (int i = 2; i < 6; i++)
    {
        rows2[i - 2] = i;
        cols2[i - 2] = 3;
    }

    updateDependencies(rows2, cols2, size, 0, 0);
    for (int i = 0; i < 4; i++)
    {
        if (cell->dependencies[i]->row != rows2[i] || cell->dependencies[i]->col != cols2[i])
        {

            printf("Test failed, here2\n");
            return 1;
        }
        else
        {
            printf("%d %d\n", cell->dependencies[i]->row, cell->dependencies[i]->col);
        }
    }

    printf("Test passed\n");
}