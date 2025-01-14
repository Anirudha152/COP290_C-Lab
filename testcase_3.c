#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"

int main()
{
    short total_rows = 15;
    short total_cols = 15;

    initStorage(total_rows, total_cols);

    short *rows1 = (short *)malloc(sizeof(short) * 4);
    short *cols1 = (short *)malloc(sizeof(short) * 4);

    for (int i = 0; i < 4; i++)
    {
        rows1[i] = i;
        cols1[i] = i;

        addDependant(7, 6, rows1[i], cols1[i]);
    }

    Cell *cell = getCell(7, 6);
    Node *temp = cell->head_dependant;
    while (temp != NULL)
    {
        printf("%d %d\n", temp->cell->row, temp->cell->col);
        temp = temp->next;
    }

    deleteDependant(7, 6, 2, 2);

    temp = cell->head_dependant;
    while (temp != NULL)
    {
        printf("%d %d\n", temp->cell->row, temp->cell->col);
        temp = temp->next;
    }

    return 0;
}