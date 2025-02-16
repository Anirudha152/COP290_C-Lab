#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "primary_storage.h"
#include "compute_unit.h"
#include "../constants.h"
#include "../data_structures/stack.h"
#include "../data_structures/set.h"

int circular_check(Cell *start_cell)
{
    clear_stack();
    clear_stack_mem();
    stack_push(start_cell);
    while (!is_stack_empty())
    {
        Cell *current = stack_top();
        Memory mem = {current->row, current->col, current->state};
        if (current->state == CLEAN || current->state == DIRTY || current->state == ZERO_ERROR)
            stack_push_mem(mem);
        current->state = DFS_IN_PROGRESS;
        if (current->dependant_count == 0)
        {
            stack_pop();
            current->state = CIRCULAR_CHECKED;
            continue;
        }
        SetIterator *iter = set_iterator_create(current->dependants);
        int done = 1;
        Cell *cell;
        while ((cell = set_iterator_next(iter)) != NULL)
        {
            if (cell->state == CLEAN || cell->state == DIRTY || cell->state == ZERO_ERROR)
            {
                stack_push(cell);
                done = 0;
            }
            else if (cell->state == DFS_IN_PROGRESS)
            {
                clear_stack();
                set_iterator_destroy(iter);
                return 0;
            }
        }
        if (done)
        {
            stack_pop();
            current->state = CIRCULAR_CHECKED;
        }
        set_iterator_destroy(iter);
    }
    return 1;
}

void clear_debris()
{
    while (stack_size_mem())
    {
        Memory mem = stack_top_mem();
        Cell *cell = get_cell(mem.row, mem.col);
        cell->state = mem.state;
        stack_pop_mem();
    }
}

pair evaluate_cell(const Cell *cell);

void mark_dirty(Cell *start_cell)
{
    clear_stack();
    stack_push(start_cell);
    while (!is_stack_empty())
    {
        Cell *current = stack_top();
        current->state = DFS_IN_PROGRESS;
        if (current->dependant_count == 0)
        {
            stack_pop();
            current->state = DIRTY;
            continue;
        }
        SetIterator *iter = set_iterator_create(current->dependants);
        int done = 1;
        Cell *cell;
        while ((cell = set_iterator_next(iter)) != NULL)
        {
            if (cell->state == CIRCULAR_CHECKED)
            {
                stack_push(cell);
                done = 0;
            }
        }
        if (done)
        {
            stack_pop();
            current->state = DIRTY;
        }
        set_iterator_destroy(iter);
    }
}

void clean_cells_forward(Cell *start_cell)
{
    clear_stack();
    stack_push(start_cell);
    while (!is_stack_empty())
    {
        Cell *current = stack_top();
        stack_pop();
        pair eval = evaluate_cell(current);
        current->state = eval.first ? CLEAN : ZERO_ERROR;
        current->value = eval.second;
        SetIterator *iter = set_iterator_create(current->dependants);
        Cell *cell;
        while ((cell = set_iterator_next(iter)) != NULL)
        {
            if (cell->state == CIRCULAR_CHECKED)
            {
                int flag = 1;
                if (cell->dependency_count == 1)
                {
                    if (cell->dependency_top_left->state != CLEAN && cell->dependency_top_left->state != ZERO_ERROR)
                    {
                        flag = 0;
                    }
                }
                if (cell->dependency_count == 2)
                {
                    if (cell->dependency_top_left->state != CLEAN && cell->dependency_top_left->state != ZERO_ERROR)
                    {
                        flag = 0;
                    }
                    else if (cell->dependency_bottom_right->state != CLEAN && cell->dependency_bottom_right->state != ZERO_ERROR)
                    {
                        flag = 0;
                    }
                }
                if (cell->dependency_count > 2)
                {
                    for (short i = cell->dependency_top_left->row; i <= cell->dependency_bottom_right->row; i++)
                    {
                        for (short j = cell->dependency_top_left->col; j <= cell->dependency_bottom_right->col; j++)
                        {
                            Cell *dependency_cell = get_cell(i, j);
                            if (dependency_cell->state != CLEAN && dependency_cell->state != ZERO_ERROR)
                            {
                                flag = 0;
                                break;
                            }
                        }
                        if (flag == 0)
                        {
                            break;
                        }
                    }
                }
                // for (int i = 0; i < cell->dependency_count; i++) {
                //     if (cell->dependencies[i]->state != CLEAN && cell->dependencies[i]->state != ZERO_ERROR) {
                //         flag = 0;
                //         break;
                //     }
                // }
                if (flag)
                {
                    stack_push(cell);
                }
            }
        }
        set_iterator_destroy(iter);
    }
}

void clean_cell(Cell *cell);

int get_cell_value(const short row, const short col)
{
    Cell *cell = get_cell(row, col);
    if (cell->state == CLEAN)
    {
        return cell->value;
    }
    clean_cell(cell);
    return cell->value;
}

pair function_compute(const Function function)
{
    int ans = 0;
    pair ret;
    const enum FunctionType type = function.type;
    if (type == MIN)
    {
        const Cell *dep = get_cell(function.range.start_row, function.range.start_col);
        if (dep->state == ZERO_ERROR)
            goto zero_error_func;
        ans = dep->value;
        for (short i = function.range.start_row; i <= function.range.end_row; i++)
        {
            for (short j = function.range.start_col; j <= function.range.end_col; j++)
            {
                dep = get_cell(i, j);
                if (dep->state == ZERO_ERROR)
                    goto zero_error_func;
                if (dep->value < ans)
                {
                    ans = dep->value;
                }
            }
        }
    }
    else if (type == MAX)
    {
        const Cell *dep = get_cell(function.range.start_row, function.range.start_col);
        if (dep->state == ZERO_ERROR)
            goto zero_error_func;
        ans = dep->value;
        for (short i = function.range.start_row; i <= function.range.end_row; i++)
        {
            for (short j = function.range.start_col; j <= function.range.end_col; j++)
            {
                dep = get_cell(i, j);
                if (dep->state == ZERO_ERROR)
                    goto zero_error_func;
                if (dep->value > ans)
                {
                    ans = dep->value;
                }
            }
        }
    }
    else if (type == AVG)
    {
        ans = 0;
        for (short i = function.range.start_row; i <= function.range.end_row; i++)
        {
            for (short j = function.range.start_col; j <= function.range.end_col; j++)
            {
                const Cell *dep = get_cell(i, j);
                if (dep->state == ZERO_ERROR)
                    goto zero_error_func;
                ans += dep->value;
            }
        }
        const int size = ((int)function.range.end_row - function.range.start_row + 1) * ((int)function.range.end_col - function.range.start_col + 1);
        ans = ans / size;
    }
    else if (type == SUM)
    {
        ans = 0;
        for (short i = function.range.start_row; i <= function.range.end_row; i++)
        {
            for (short j = function.range.start_col; j <= function.range.end_col; j++)
            {
                const Cell *dep = get_cell(i, j);
                if (dep->state == ZERO_ERROR)
                    goto zero_error_func;
                ans += dep->value;
            }
        }
    }
    else if (type == STDEV)
    {
        float temp = 0;
        float temp_sq = 0;
        for (short i = function.range.start_row; i <= function.range.end_row; i++)
        {
            for (short j = function.range.start_col; j <= function.range.end_col; j++)
            {
                const Cell *dep = get_cell(i, j);
                if (dep->state == ZERO_ERROR)
                    goto zero_error_func;
                temp += dep->value;
            }
        }
        const int size = (function.range.end_row - function.range.start_row + 1) * (function.range.end_col - function.range.start_col + 1);
        const float avg = temp / (float)size;

        for (short i = function.range.start_row; i <= function.range.end_row; i++)
        {
            for (short j = function.range.start_col; j <= function.range.end_col; j++)
            {
                temp_sq += ((float)get_raw_value(i, j) - avg) * ((float)get_raw_value(i, j) - avg);
            }
        }

        ans = (int)sqrt(temp_sq / (double)size);
    }
    else if (type == SLEEP)
    {
        if (function.value.type == INTEGER)
        {
            // sleep(function.value.value > 0 ? function.value.value : 0);
            ans = function.value.value;
        }
        else if (function.value.type == CELL_REFERENCE)
        {
            const Cell *dep = function.value.cell;
            if (dep->state == ZERO_ERROR)
                goto zero_error_func;
            ans = get_raw_value(function.value.cell->row, function.value.cell->col);
            // sleep(ans > 0 ? ans : 0);
        }
        else
        {
            printf("Value holds Error State");
            exit(1);
        }
    }
    ret.first = 1;
    ret.second = ans;
    return ret;
zero_error_func:
    ret.first = 0;
    ret.second = 0;
    return ret;
}

pair evaluate_cell(const Cell *cell)
{
    const Expression expression = cell->expression;
    int eval = 0;
    if (expression.type == VALUE)
    {
        const Value value = expression.value;
        if (value.type == INTEGER)
        {
            eval = value.value;
        }
        else if (value.type == CELL_REFERENCE)
        {
            eval = value.cell->value;
            if (value.cell->state == ZERO_ERROR)
                goto zero_error;
        }
        else
        {
            printf("Value holds Error State");
            exit(1);
        }
    }
    else if (expression.type == ARITHMETIC)
    {
        const Arithmetic arithmetic = expression.arithmetic;
        const Value value1 = arithmetic.value1;
        const Value value2 = arithmetic.value2;
        int val1 = 0;
        int val2 = 0;
        if (value1.type == INTEGER)
        {
            val1 = value1.value;
        }
        else if (value1.type == CELL_REFERENCE)
        {
            val1 = value1.cell->value;
            if (value1.cell->state == ZERO_ERROR)
                goto zero_error;
        }
        else
        {
            printf("Value holds Error State");
            exit(1);
        }

        if (value2.type == INTEGER)
        {
            val2 = value2.value;
        }
        else if (value2.type == CELL_REFERENCE)
        {
            val2 = value2.cell->value;
            if (value2.cell->state == ZERO_ERROR)
                goto zero_error;
        }
        else
        {
            printf("Value holds Error State");
            exit(1);
        }

        if (arithmetic.type == ADD)
        {
            eval = val1 + val2;
        }
        else if (arithmetic.type == SUBTRACT)
        {
            eval = val1 - val2;
        }
        else if (arithmetic.type == MULTIPLY)
        {
            eval = val1 * val2;
        }
        else if (arithmetic.type == DIVIDE)
        {
            if (val2 == 0)
            {
                goto zero_error;
            }
            eval = val1 / val2;
        }
    }
    else if (expression.type == FUNCTION)
    {
        return function_compute(expression.function);
    }
    return (pair){1, eval};
zero_error:
    eval = 0;
    return (pair){0, eval};
}

void clean_cell(Cell *cell)
{
    if (cell->state == CLEAN)
    {
        return;
    }
    clear_stack();
    stack_push(cell);
    while (!is_stack_empty())
    {
        Cell *current_cell = stack_top();
        if (current_cell->state == CLEAN)
        {
            stack_pop();
            continue;
        }
        int need_to_process_dependencies = 0;
        if (current_cell->dependency_count == 1)
        {
            if (current_cell->dependency_top_left->state == DIRTY)
            {
                stack_push(current_cell->dependency_top_left);
                need_to_process_dependencies = 1;
            }
        }
        if (current_cell->dependency_count == 2)
        {
            if (current_cell->dependency_top_left->state == DIRTY)
            {
                stack_push(current_cell->dependency_top_left);
                need_to_process_dependencies = 1;
            }
            else if (current_cell->dependency_bottom_right->state == DIRTY)
            {
                stack_push(current_cell->dependency_bottom_right);
                need_to_process_dependencies = 1;
            }
        }
        if (current_cell->dependency_count > 2)
        {
            for (short i = current_cell->dependency_top_left->row; i <= current_cell->dependency_bottom_right->row; i++)
            {
                for (short j = current_cell->dependency_top_left->col; j <= current_cell->dependency_bottom_right->col; j++)
                {
                    Cell *dependency_cell = get_cell(i, j);
                    if (dependency_cell->state == DIRTY)
                    {
                        stack_push(dependency_cell);
                        need_to_process_dependencies = 1;
                        break;
                    }
                }
                if (need_to_process_dependencies == 1)
                {
                    break;
                }
            }
        }
        // for (int i = 0; i < current_cell->dependency_count; i++) {
        //     if (current_cell->dependencies[i]->state == DIRTY) {
        //         stack_push(current_cell->dependencies[i]);
        //         need_to_process_dependencies = 1;
        //         break;
        //     }
        // }
        if (need_to_process_dependencies)
        {
            continue;
        }
        stack_pop();
        const pair eval = evaluate_cell(current_cell);
        current_cell->state = eval.first ? CLEAN : ZERO_ERROR;
        current_cell->value = eval.second;
    }
}

void copy_dependencies(Cell **dependencies, const size_t dependencies_count, short *rows, short *cols)
{
    for (int i = 0; i < dependencies_count; i++)
    {
        rows[i] = dependencies[i]->row;
        cols[i] = dependencies[i]->col;
    }
}

int handle_circular_connection(Cell *cell, short *rows_prev, short *cols_prev, const size_t dependencies_count, const Expression expression)
{
    if (!circular_check(cell))
    {
        clear_debris();
        // delete current cell as a dependant from its new dependencies
        if (cell->dependency_count == 1)
        {
            delete_dependant(cell->dependency_top_left->row, cell->dependency_top_left->col, cell->row, cell->col);
        }
        if (cell->dependency_count == 2)
        {
            delete_dependant(cell->dependency_top_left->row, cell->dependency_top_left->col, cell->row, cell->col);
            delete_dependant(cell->dependency_bottom_right->row, cell->dependency_bottom_right->col, cell->row, cell->col);
        }
        if (cell->dependency_count > 2)
        {
            for (short i = cell->dependency_top_left->row; i <= cell->dependency_bottom_right->row; i++)
            {
                for (short j = cell->dependency_top_left->col; j <= cell->dependency_bottom_right->col; j++)
                {
                    Cell *dependency_cell = get_cell(i, j);
                    delete_dependant(dependency_cell->row, dependency_cell->col, cell->row, cell->col);
                }
            }
        }
        // for (int i = 0; i < cell->dependency_count; i++) {
        //     delete_dependant(cell->dependencies[i]->row, cell->dependencies[i]->col, cell->row, cell->col);
        // }
        // add current cell as a dependant to its old dependencies
        for (int i = 0; i < dependencies_count; i++)
        {
            add_dependant(rows_prev[i], cols_prev[i], cell->row, cell->col);
        }
        // restore the dependencies from before
        cell->dependency_count = 0;
        if (cell->dependency_top_left != NULL)
            free(cell->dependency_top_left);
        if (cell->dependency_bottom_right != NULL)
            free(cell->dependency_bottom_right);
        cell->dependency_top_left = NULL;
        cell->dependency_bottom_right = NULL;
        for (int i = 0; i < dependencies_count; i++)
        {
            update_dependencies(rows_prev, cols_prev, dependencies_count, cell->row, cell->col);
        }
        free(rows_prev);
        free(cols_prev);
        return 0;
    }
    cell->expression = expression;
    if (LAZY_EVALUATION)
        mark_dirty(cell);
    else
        clean_cells_forward(cell);
    free(rows_prev);
    free(cols_prev);
    return 1;
}

int set_expression(const short row, const short col, const Expression expression)
{
    Cell *cell = get_cell(row, col);
    Cell **dependency_top_left = cell->dependency_top_left;
    Cell **dependency_bottom_right = cell->dependency_bottom_right;

    const size_t dependencies_count = cell->dependency_count;
    short *rows_prev = malloc(dependencies_count * sizeof(short)), *cols_prev = malloc(dependencies_count * sizeof(short));
    if (rows_prev == NULL || cols_prev == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    Cell *dependencies[2];
    dependencies[0] = cell->dependency_top_left;
    dependencies[1] = cell->dependency_bottom_right;
    copy_dependencies(dependencies, dependencies_count, rows_prev, cols_prev);
    for (int i = 0; i < min(dependencies_count, 2); i++)
    {
        delete_dependant(dependencies[i]->row, dependencies[i]->col, row, col);
    }

    if (expression.type == VALUE)
    {
        if (expression.value.type == INTEGER)
        {
            if (cell->dependency_top_left != NULL)
            {
                free(cell->dependency_top_left);
                cell->dependency_top_left = NULL;
            }
            if (cell->dependency_bottom_right != NULL)
            {
                free(cell->dependency_bottom_right);
                cell->dependency_bottom_right = NULL;
            }
            cell->dependency_count = 0;
        }
        else if (expression.value.type == CELL_REFERENCE)
        {
            const short rows[1] = {expression.value.cell->row};
            const short cols[1] = {expression.value.cell->col};
            update_dependencies(rows, cols, 1, row, col);
            add_dependant(expression.value.cell->row, expression.value.cell->col, row, col);
        }
        else
        {
            printf("Value holds Error State");
            exit(1);
        }
    }
    else if (expression.type == ARITHMETIC)
    {
        const enum ValueType type1 = expression.arithmetic.value1.type;
        const enum ValueType type2 = expression.arithmetic.value2.type;
        if (type1 == INTEGER && type2 == INTEGER)
        {
            cell->dependency_count = 0;
            if (cell->dependency_top_left != NULL)
            {
                free(cell->dependency_top_left);
                cell->dependency_top_left = NULL;
            }
            if (cell->dependency_bottom_right != NULL)
            {
                free(cell->dependency_bottom_right);
                cell->dependency_bottom_right = NULL;
            }
        }
        else if ((type1 != CELL_REFERENCE) != (type2 != CELL_REFERENCE))
        {
            const short row_ = type1 == CELL_REFERENCE ? expression.arithmetic.value1.cell->row : expression.arithmetic.value2.cell->row;
            const short col_ = type1 == CELL_REFERENCE ? expression.arithmetic.value1.cell->col : expression.arithmetic.value2.cell->col;
            const short rows[1] = {row_};
            const short cols[1] = {col_};
            update_dependencies(rows, cols, 1, row, col);
            add_dependant(row_, col_, row, col);
        }
        else
        {
            const short rows[2] = {expression.arithmetic.value1.cell->row, expression.arithmetic.value2.cell->row};
            const short cols[2] = {expression.arithmetic.value1.cell->col, expression.arithmetic.value2.cell->col};
            update_dependencies(rows, cols, 2, row, col);
            add_dependant(expression.arithmetic.value1.cell->row, expression.arithmetic.value1.cell->col, row, col);
            add_dependant(expression.arithmetic.value2.cell->row, expression.arithmetic.value2.cell->col, row, col);
        }
    }
    else if (expression.type == FUNCTION && expression.function.type != SLEEP)
    {
        const int size = (expression.function.range.end_row - expression.function.range.start_row + 1) * (expression.function.range.end_col - expression.function.range.start_col + 1);
        short *rows = malloc(size * sizeof(short));
        short *cols = malloc(size * sizeof(short));
        rows[0] = expression.function.range.start_row;
        rows[1] = expression.function.range.end_row;
        cols[0] = expression.function.range.start_col;
        cols[1] = expression.function.range.end_col;
        for (short i = expression.function.range.start_row; i <= expression.function.range.end_row; i++)
        {
            for (short j = expression.function.range.start_col; j <= expression.function.range.end_col; j++)
            {
                add_dependant(i, j, row, col);
            }
        }
        update_dependencies(rows, cols, size, row, col);
        free(cols);
        free(rows);
    }
    else if (expression.type == FUNCTION && expression.function.type == SLEEP)
    {
        if (expression.function.value.type == INTEGER)
        {
            cell->dependency_count = 0;
            if (cell->dependency_top_left != NULL)
            {
                free(cell->dependency_top_left);
                cell->dependency_top_left = NULL;
            }
            if (cell->dependency_bottom_right != NULL)
            {
                free(cell->dependency_bottom_right);
                cell->dependency_bottom_right = NULL;
            }
        }
        else if (expression.function.value.type == CELL_REFERENCE)
        {
            add_dependant(expression.function.value.cell->row, expression.function.value.cell->col, row, col);
            const short rows[1] = {expression.function.value.cell->row};
            const short cols[1] = {expression.function.value.cell->col};
            update_dependencies(rows, cols, 1, row, col);
        }
    }
    return handle_circular_connection(cell, rows_prev, cols_prev, dependencies_count, expression);
}

int set_value_expression(const short row, const short col, const Value value)
{
    Expression expression;
    expression.type = VALUE;
    expression.value = value;
    return set_expression(row, col, expression);
}

int set_arithmetic_expression(const short row, const short col, const Arithmetic arithmetic)
{
    Expression expression;
    expression.type = ARITHMETIC;
    expression.arithmetic = arithmetic;
    return set_expression(row, col, expression);
}

int set_function_expression(const short row, const short col, const Function function)
{
    Expression expression;
    expression.type = FUNCTION;
    expression.function = function;
    return set_expression(row, col, expression);
}

// int set_sleep_expression(const short row, const short col, const Value value) {
//     return set_expression(row, col, 2, value, (Value) {0, 0, NULL}, -1, 5, (Range) {0, -1, -1, -1, -1});
// }