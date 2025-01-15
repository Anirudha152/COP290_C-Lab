#include <stdio.h>
#include <stdlib.h>
#include "primary_storage.h"

int getValue(short row, short col)
{
    Cell *cell = getCell(row, col);
    if (cell->state == 0)
    {
        return cell->value;
    }
    else
    {
    }
}