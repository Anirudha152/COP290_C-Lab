#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "primary_storage.h"
#include "compute_unit.h"
#include "../data_structures/stack.h"
#include "../data_structures/set.h"

int circular_check(Cell *start_cell) {
    clear_stack();
    clear_stack_mem();
    stack_push(start_cell);
    while (!is_stack_empty()) {
        Cell *current = stack_top();
        Memory mem = {current->row, current->col, current->state};
        stack_push_mem(mem);
        current->state = 3;
        if (current->dependant_count == 0) {
            stack_pop();
            current->state = 4;
            continue;
        }
        SetIterator *iter = set_iterator_create(current->dependants);
        int done = 1;
        Cell *cell;
        while ((cell = set_iterator_next(iter)) != NULL) {
            if (cell->state < 2) {
                stack_push(cell);
                done = 0;
            } else if (cell->state == 3) {
                clear_stack();
                set_iterator_destroy(iter);
                return 0;
            }
        }
        if (done) {
            stack_pop();
            current->state = 4;
        }
        set_iterator_destroy(iter);
    }
    return 1;
}

void clear_debris() {
    while (stack_size_mem()) {
        Memory mem = stack_top_mem();
        Cell* cell = get_cell(mem.row, mem.col);
        cell->state = mem.state;
        stack_pop_mem();
    }
}

void mark_dirty(Cell *start_cell) {
    clear_stack();
    stack_push(start_cell);
    while (!is_stack_empty()) {
        Cell *current = stack_top();
        current->state = 3;
        if (current->dependant_count == 0) {
            stack_pop();
            current->state = 1;
            continue;
        }
        SetIterator *iter = set_iterator_create(current->dependants);
        int done = 1;
        Cell *cell;
        while ((cell = set_iterator_next(iter)) != NULL) {
            if (cell->state != 3 && cell->state != 1) {
                stack_push(cell);
                done = 0;
            }
        }
        if (done) {
            stack_pop();
            current->state = 1;
        }
        set_iterator_destroy(iter);
    }
}

void clean_cell(Cell *cell);

int get_cell_value(const short row, const short col) {
    Cell *cell = get_cell(row, col);
    if (cell->state == 0) {
        return cell->value;
    }
    clean_cell(cell);
    return cell->value;
}

pair function_compute(const short type, const Range range) {
    int ans;
    pair ret;
    if (type == 0) {
        const Cell *dep = get_cell(range.start_row, range.start_col);
        if (dep->state == 9) goto zero_error_func;
        ans = dep->value;
        for (short i = range.start_row; i <= range.end_row; i++) {
            for (short j = range.start_col; j <= range.end_col; j++) {
                dep = get_cell(i, j);
                if (dep->state == 9) goto zero_error_func;
                if (dep->value < ans) {
                    ans = dep->value;
                }
            }
        }
    } else if (type == 1) {
        const Cell *dep = get_cell(range.start_row, range.start_col);
        if (dep->state == 9) goto zero_error_func;
        ans = dep->value;
        for (short i = range.start_row; i <= range.end_row; i++) {
            for (short j = range.start_col; j <= range.end_col; j++) {
                dep = get_cell(i, j);
                if (dep->state == 9) goto zero_error_func;
                if (dep->value > ans) {
                    ans = dep->value;
                }
            }
        }
    } else if (type == 2) {
        ans = 0;
        for (short i = range.start_row; i <= range.end_row; i++) {
            for (short j = range.start_col; j <= range.end_col; j++) {
                const Cell *dep = get_cell(i, j);
                if (dep->state == 9) goto zero_error_func;
                ans += dep->value;
            }
        }
        const int size = (range.end_row - range.start_row + 1) * (range.end_col - range.start_col + 1);
        ans = ans / size;
    } else if (type == 3) {
        ans = 0;
        for (short i = range.start_row; i <= range.end_row; i++) {
            for (short j = range.start_col; j <= range.end_col; j++) {
                const Cell *dep = get_cell(i, j);
                if (dep->state == 9) goto zero_error_func;
                ans += dep->value;
            }
        }
    } else if (type == 4) {
        float temp = 0;
        float temp_sq = 0;
        for (short i = range.start_row; i <= range.end_row; i++) {
            for (short j = range.start_col; j <= range.end_col; j++) {
                const Cell *dep = get_cell(i, j);
                if (dep->state == 9) goto zero_error_func;
                temp += dep->value;
            }
        }
        const int size = (range.end_row - range.start_row + 1) * (range.end_col - range.start_col + 1);
        const float avg = temp / (float)size;

        for (short i = range.start_row; i <= range.end_row; i++) {
            for (short j = range.start_col; j <= range.end_col; j++) {
                temp_sq += ((float) get_raw_value(i, j) - avg) * ((float) get_raw_value(i, j) - avg);
            }
        }

        ans = (int)sqrt(temp_sq / (double)size);
    } else {
        ans = 0;
    }
    ret.first = 1;
    ret.second = ans;
    return ret;
zero_error_func:
    ret.first = 0;
    ret.second = 0;
    return ret;
}

pair sleep_compute(const Value value) {
    if (value.type == 0) {
        sleep(value.value > 0 ? value.value : 0);
        const pair ret = {1, value.value};
        return ret;
    }
    if (value.type == 1) {
        const Cell* dep = value.cell;
        if (dep->state == 9) {
            const pair ret = {0, 0};
            return ret;
        }
        const int val = get_raw_value(value.cell->row, value.cell->col);
        sleep(val > 0 ? val : 0);
        const pair ret = {1, val};
        return ret;
    }
    const pair ret = {1, 0};
    return ret;
}

void clean_cell(Cell *cell) {
    if (cell->state == 0) {
        return;
    }
    clear_stack();
    stack_push(cell);
    while (!is_stack_empty()) {
        Cell *current_cell = stack_top();
        if (current_cell->state == 0) {
            stack_pop();
            continue;
        }
        int need_to_process_dependencies = 0;
        for (int i = 0; i < current_cell->dependency_count; i++) {
            if (current_cell->dependencies[i]->state == 1) {
                stack_push(current_cell->dependencies[i]);
                need_to_process_dependencies = 1;
                break;
            }
        }
        if (need_to_process_dependencies) {
            continue;
        }
        stack_pop();
        const Expression formula = current_cell->formula;
        if (formula.type == 0) {
            const Value value = formula.value1;
            if (value.type == 0) {
                current_cell->value = value.value;
            } else {
                current_cell->value = value.cell->value;
                if (value.cell->state == 9) goto zero_error;
            }
        } else if (formula.type == 1) {
            const Value value1 = formula.value1;
            const Value value2 = formula.value2;
            int val1;
            int val2;
            if (value1.type == 0) {
                val1 = value1.value;
            } else {
                val1 = value1.cell->value;
                if (value1.cell->state == 9) goto zero_error;
            }

            if (value2.type == 0) {
                val2 = value2.value;
            } else {
                val2 = value2.cell->value;
                if (value2.cell->state == 9) goto zero_error;
            }

            if (formula.operation == 0) {
                current_cell->value = val1 + val2;
            } else if (formula.operation == 1) {
                current_cell->value = val1 - val2;
            } else if (formula.operation == 2) {
                current_cell->value = val1 * val2;
            } else if (formula.operation == 3) {
                if (val2 == 0) {
                    goto zero_error;
                }
                current_cell->value = val1 / val2;
            }
        } else if (formula.type == 2) {
            const Function function = formula.function;
            if (function.type != 5) {
                const pair ret = function_compute(function.type, function.range);
                if (!ret.first) goto zero_error;
                current_cell->value = ret.second;
            } else {
                const pair ret = sleep_compute(formula.value1);
                if (!ret.first) goto zero_error;
                current_cell->value = ret.second;
            }
        }
        current_cell->state = 0;
        continue;
    zero_error:
        current_cell->value = 0;
        current_cell->state = 9;
    }
}

void copy_dependencies(Cell** dependencies, const size_t dependencies_count, short* rows, short* cols) {
    for (int i = 0; i < dependencies_count; i++) {
        rows[i] = dependencies[i]->row;
        cols[i] = dependencies[i]->col;
    }
}

void handle_circular_connection(int circular_stat, Cell *cell, short *rows_prev, short *cols_prev, const size_t dependencies_count, Expression formula) {
    if (!circular_stat) {
        clear_debris();
        // delete current cell as a dependant from its new dependencies
        for (int i = 0; i < cell->dependency_count; i++) {
            delete_dependant(cell->dependencies[i]->row, cell->dependencies[i]->col, cell->row, cell->col);
        }
        // add current cell as a dependant to its old dependencies
        for (int i = 0; i < dependencies_count; i++) {
            add_dependant(rows_prev[i], cols_prev[i], cell->row, cell->col);
        }
        // restore the dependencies from before
        cell->dependency_count = 0;
        free(cell->dependencies);
        cell->dependencies = NULL;
        for (int i = 0; i < dependencies_count; i++) {
            update_dependencies(rows_prev, cols_prev, dependencies_count, cell->row, cell->col);
        }
    } else {
        cell->formula = formula;
        mark_dirty(cell);
    }
    free(rows_prev);
    free(cols_prev);
}

int set_expression(const short row, const short col, const short expression_type, const Value value1, const Value value2, const short operation, const short function_type, const Range range) {
    Cell *cell = get_cell(row, col);
    Expression formula = cell->formula;
    Cell **dependencies = cell->dependencies;
    const size_t dependencies_count = cell->dependency_count;
    short *rows_prev = malloc(dependencies_count * sizeof(short)), *cols_prev = malloc(dependencies_count * sizeof(short));
    if (rows_prev == NULL || cols_prev == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    copy_dependencies(dependencies, dependencies_count, rows_prev, cols_prev);
    for (int i = 0; i < dependencies_count; i++) {
        delete_dependant(dependencies[i]->row, dependencies[i]->col, row, col);
    }

    formula.type = expression_type;
    formula.value1 = value1;
    formula.value2 = value2;
    formula.operation = operation;
    formula.function.type = function_type;
    formula.function.range = range;
    int circular_stat = 1;
    if (expression_type == 0) {
        if (value1.type == 0) {
            free(cell->dependencies);
            cell->dependencies = NULL;
            cell->dependency_count = 0;
        } else {
            const short rows[1] = {value1.cell->row};
            const short cols[1] = {value1.cell->col};
            update_dependencies(rows, cols, 1, row, col);
            add_dependant(value1.cell->row, value1.cell->col, row, col);
            circular_stat = circular_check(cell);
        }
    } else if (expression_type == 1) {
        if (value1.type + value2.type == 0) {
            free(cell->dependencies);
            cell->dependencies = NULL;
            cell->dependency_count = 0;
        } else if (value1.type + value2.type == 1) {
            const short row_ = value1.type ? value1.cell->row : value2.cell->row;
            const short col_ = value1.type ? value1.cell->col : value2.cell->col;
            const short rows[1] = {row_};
            const short cols[1] = {col_};
            update_dependencies(rows, cols, 1, row, col);
            add_dependant(row_, col_, row, col);
        } else {
            const short rows[2] = {value1.cell->row, value2.cell->row};
            const short cols[2] = {value1.cell->col, value2.cell->col};
            update_dependencies(rows, cols, 2, row, col);
            add_dependant(value1.cell->row, value1.cell->col, row, col);
            add_dependant(value2.cell->row, value2.cell->col, row, col);
        }
        circular_stat = value1.type + value2.type ? circular_check(cell) : 0;
    } else if (expression_type == 2 && function_type != 5) {
        const int size = (range.end_row - range.start_row + 1) * (range.end_col - range.start_col + 1);
        short *rows = malloc(size * sizeof(short));
        short *cols = malloc(size * sizeof(short));
        for (short i = range.start_row; i <= range.end_row; i++) {
            for (short j = range.start_col; j <= range.end_col; j++) {
                rows[(i - range.start_row) * (range.end_col - range.start_col + 1) + (j - range.start_col)] = i;
                cols[(i - range.start_row) * (range.end_col - range.start_col + 1) + (j - range.start_col)] = j;
                add_dependant(i, j, row, col);
            }
        }
        update_dependencies(rows, cols, size, row, col);
        circular_stat = circular_check(cell);
        free(cols);
        free(rows);
    } else if (expression_type == 2 && function_type == 5) {
        if (value1.type == 0) {
            free(cell->dependencies);
            cell->dependencies = NULL;
            cell->dependency_count = 0;
        } else if (value1.type == 1) {
            add_dependant(value1.cell->row, value1.cell->col, row, col);
            const short rows[1] = {value1.cell->row};
            const short cols[1] = {value1.cell->col};
            update_dependencies(rows, cols, 1, row, col);
            circular_stat = circular_check(cell);
        }
    }
    handle_circular_connection(circular_stat, cell, rows_prev, cols_prev, dependencies_count, formula);
    return circular_stat;
}

int set_value_expression(const short row, const short col, const Value value) {
    return set_expression(row, col, 0, value, (Value) {0, 0, NULL}, -1, -1, (Range) {0, -1, -1, -1, -1});
}

int set_arithmetic_expression(const short row, const short col, const Value value1, const Value value2, const short operation) {
    return set_expression(row, col, 1, value1, value2, operation, -1, (Range) {0, -1, -1, -1, -1});
}

int set_function_expression(const short row, const short col, const short type, const Range range) {
    return set_expression(row, col, 2, (Value) {0, 0, NULL}, (Value) {0, 0, NULL}, -1, type, range);
}

int set_sleep_expression(const short row, const short col, const Value value) {
    return set_expression(row, col, 2, value, (Value) {0, 0, NULL}, -1, 5, (Range) {0, -1, -1, -1, -1});
}