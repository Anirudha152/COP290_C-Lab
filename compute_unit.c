#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "primary_storage.h"

void markDirty(Cell *cell)
{

    if (cell->state == 4)
    {
        printf("Circular dependency\n");
        exit(1);
    }
    cell->state = 4;
    Node *temp = cell->head_dependant;
    while (temp != NULL)
    {

        markDirty(temp->cell);
        temp = temp->next;
    }
    cell->state = 1;
}
void cleanCell(Cell *cell);
int getValue(short row, short col)
{
    Cell *cell = getCell(row, col);
    if (cell->state == 0)
    {
        return cell->value;
    }
    else
    {
        cleanCell(cell);
        return cell->value;
    }
}

int functionCompute(short type, Range range)
{
    int ans;
    if (type == 0)
    {
        ans = getValue(range.start_row, range.start_col);
        for (short i = range.start_row; i <= range.end_row; i++)
        {
            for (short j = range.start_col; j <= range.end_col; j++)
            {
                if (getValue(i, j) < ans)
                {
                    ans = getValue(i, j);
                }
            }
        }
    }

    if (type == 1)
    {
        ans = getValue(range.start_row, range.start_col);
        for (short i = range.start_row; i <= range.end_row; i++)
        {
            for (short j = range.start_col; j <= range.end_col; j++)
            {
                if (getValue(i, j) > ans)
                {
                    ans = getValue(i, j);
                }
            }
        }
    }

    if (type == 2)
    {
        ans = 0;
        for (short i = range.start_row; i <= range.end_row; i++)
        {
            for (short j = range.start_col; j <= range.end_col; j++)
            {
                ans += getValue(i, j);
            }
        }
        int size = (int)(range.end_row - range.start_row + 1) * (int)(range.end_col - range.start_col + 1);
        ans = ans / size;
    }

    if (type == 3)
    {
        ans = 0;
        for (short i = range.start_row; i <= range.end_row; i++)
        {
            for (short j = range.start_col; j <= range.end_col; j++)
            {
                ans += getValue(i, j);
            }
        }
    }

    if (type == 4)
    {
        float temp = 0;
        float temp_sq = 0;
        for (short i = range.start_row; i <= range.end_row; i++)
        {
            for (short j = range.start_col; j <= range.end_col; j++)
            {
                temp += getValue(i, j);
            }
        }
        int size = (int)(range.end_row - range.start_row + 1) * (int)(range.end_col - range.start_col + 1);
        float avg = temp / size;

        for (short i = range.start_row; i <= range.end_row; i++)
        {
            for (short j = range.start_col; j <= range.end_col; j++)
            {
                temp_sq += ((float)getValue(i, j) - avg) * ((float)getValue(i, j) - avg);
            }
        }

        ans = sqrt(temp_sq / size);
    }

    return ans;
}

void cleanCell(Cell *cell)
{
    if (cell->state == 0)
    {
        return;
    }
    // if (cell->state == 2)
    // {
    //     printf("Circular dependency\n");
    //     exit(1);
    // }
    // cell->state = 2;
    Cell **dependencies = cell->dependencies;
    int dependencies_count = cell->dependency_count;
    for (int i = 0; i < dependencies_count; i++)
    {
        if (dependencies[i]->state == 1)
        {
            cleanCell(dependencies[i]);
        }
    }

    Expression formula = cell->formula;
    if (formula.type == 0)
    {
        Value value = formula.value1;
        if (value.type == 0)
        {
            cell->value = value.value;
        }
        else
        {
            cell->value = getValue(value.cell->row, value.cell->col);
        }
    }

    if (formula.type == 1)
    {
        Value value1 = formula.value1;
        Value value2 = formula.value2;

        int val1;
        int val2;

        if (value1.type == 0)
        {
            val1 = value1.value;
        }
        else
        {
            val1 = getValue(value1.cell->row, value1.cell->col);
        }

        if (value2.type == 0)
        {
            val2 = value2.value;
        }
        else
        {
            val2 = getValue(value2.cell->row, value2.cell->col);
        }

        if (formula.operation == 0)
        {
            cell->value = val1 + val2;
        }
        else if (formula.operation == 1)
        {
            cell->value = val1 - val2;
        }
        else if (formula.operation == 2)
        {
            cell->value = val1 * val2;
        }
        else if (formula.operation == 3)
        {
            if (val2 == 0)
            {
                printf("Division by zero\n");
                exit(1);
            }
            cell->value = val1 / val2;
        }
    }

    if (formula.type == 2)
    {
        Function function = formula.function;
        cell->value = functionCompute(function.type, function.range);
    }

    cell->state = 0;
    return;
}

void setValueExpression(short row, short col, Value value)
{
    Cell *cell = getCell(row, col);
    Expression oldFormula = cell->formula;
    Cell **dependencies = cell->dependencies;
    int dependencies_count = cell->dependency_count;
    for (int i = 0; i < dependencies_count; i++)
    {
        deleteDependant(dependencies[i]->row, dependencies[i]->col, row, col);
    }

    oldFormula.type = 0;
    oldFormula.value1 = value;
    if (value.type == 0)
    {
        free(cell->dependencies);
        cell->dependencies = NULL;
        cell->dependency_count = 0;
    }

    else
    {
        short rows[1] = {value.cell->row};
        short cols[1] = {value.cell->col};
        updateDependencies(rows, cols, 1, row, col);

        addDependant(value.cell->row, value.cell->col, row, col);
    }

    cell->formula = oldFormula;

    markDirty(cell);
}

void setArithmeticExpression(short row, short col, Value value1, Value value2, short operation)
{
    Cell *cell = getCell(row, col);
    Expression oldFormula = cell->formula;

    Cell **dependencies = cell->dependencies;
    int dependencies_count = cell->dependency_count;
    for (int i = 0; i < dependencies_count; i++)
    {
        deleteDependant(dependencies[i]->row, dependencies[i]->col, row, col);
    }

    oldFormula.type = 1;
    oldFormula.value1 = value1;
    oldFormula.value2 = value2;
    oldFormula.operation = operation;

    if (value1.type == 1)
        addDependant(value1.cell->row, value1.cell->col, row, col);
    if (value2.type == 1)
        addDependant(value2.cell->row, value2.cell->col, row, col);

    if (value1.type == 0)
    {
        if (value2.type == 0)
        {
            free(cell->dependencies);
        }
        else
        {
            short rows[1] = {value2.cell->row};
            short cols[1] = {value2.cell->col};
            updateDependencies(rows, cols, 1, row, col);
        }
    }
    else
    {
        if (value2.type == 0)
        {
            short rows[1] = {value1.cell->row};
            short cols[1] = {value1.cell->col};
            updateDependencies(rows, cols, 1, row, col);
        }
        else
        {
            short rows[2] = {value1.cell->row, value2.cell->row};
            short cols[2] = {value1.cell->col, value2.cell->col};
            updateDependencies(rows, cols, 2, row, col);
        }
    }

    cell->formula = oldFormula;

    markDirty(cell);
}

void setFunctionExpression(short row, short col, short type, Range range)
{
    Cell *cell = getCell(row, col);
    Expression oldFormula = cell->formula;

    Cell **dependencies = cell->dependencies;
    int dependencies_count = cell->dependency_count;
    for (int i = 0; i < dependencies_count; i++)
    {
        deleteDependant(dependencies[i]->row, dependencies[i]->col, row, col);
    }

    Function function;
    function.type = type;
    function.range = range;
    oldFormula.function = function;
    oldFormula.type = 2;

    int size = (int)(range.end_row - range.start_row + 1) * (int)(range.end_col - range.start_col + 1);

    short rows[size];
    short cols[size];

    for (short i = range.start_row; i <= range.end_row; i++)
    {
        for (short j = range.start_col; j <= range.end_col; j++)
        {
            rows[(int)(i - range.start_row) * (int)(range.end_col - range.start_col + 1) + (int)(j - range.start_col)] = i;
            cols[(int)(i - range.start_row) * (int)(range.end_col - range.start_col + 1) + (int)(j - range.start_col)] = j;

            addDependant(i, j, row, col);
        }
    }

    updateDependencies(rows, cols, size, row, col);

    cell->formula = oldFormula;

    markDirty(cell);
}
