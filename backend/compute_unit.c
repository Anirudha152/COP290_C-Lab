#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "primary_storage.h"
#include "compute_unit.h"
#include "../constants.h"
#include "../data_structures/stack.h"
#include "../data_structures/set.h"

int circular_check(const int start_cell_index) {
    clear_stack();
    clear_stack_mem();
    stack_push(start_cell_index);
    while (!is_stack_empty()) {
        const int current_index = stack_top();
        Cell *current = get_cell(current_index);
        const Memory mem = {current_index, current->cell_state};
        if (current->cell_state == 0 || current->cell_state == 3)
            stack_push_mem(mem);
        current->cell_state = 1;
        // if (set_size(current->dependants) == 0) {
        //     stack_pop();
        //     current->cell_state = 2;
        //     continue;
        // }
        if (current->dependants_type == ArrayForm) {
            if (current->dependants_array->size == 0) {
                stack_pop();
                current->cell_state = 2;
                continue;
            }
        } else {
            if (set_size(current->dependants_set) == 0) {
                stack_pop();
                current->cell_state = 2;
                continue;
            }
        }
        if (current->dependants_type == ArrayForm) {
            int done = 1;
            int cell_index;
            for (int i = 0; i < current->dependants_array->size; i++) {
                cell_index = current->dependants_array->dependants_cells[i];
                const Cell *cell = get_cell(cell_index);
                if (cell->cell_state == 0 || cell->cell_state == 3) {
                    stack_push(cell_index);
                    done = 0;
                    break;
                }
                if (cell->cell_state == 1) {
                    clear_stack();
                    return 0;
                }
            }
            if (done) {
                stack_pop();
                current->cell_state = 2;
            }
        } else {
            SetIterator *iter = set_iterator_create(current->dependants_set);
            int done = 1;
            int cell_index;
            while ((cell_index = set_iterator_next(iter)) != -1) {
                const Cell *cell = get_cell(cell_index);
                if (cell->cell_state == 0 || cell->cell_state == 3) {
                    stack_push(cell_index);
                    done = 0;
                    break;
                }
                if (cell->cell_state == 1) {
                    clear_stack();
                    set_iterator_destroy(iter);
                    return 0;
                }
            }
            if (done) {
                stack_pop();
                current->cell_state = 2;
            }
            set_iterator_destroy(iter);
        }
    }
    return 1;
}

void clear_debris() {
    while (stack_size_mem()) {
        const Memory mem = stack_top_mem();
        Cell *cell = get_cell(mem.cell_index);
        cell->cell_state = mem.state;
        stack_pop_mem();
    }
}

pair evaluate_cell(const Cell *cell);

int get_dependency_count(const Cell *cell) {
    int count = 0;
    if (cell->expression_type == 0) {
        count = cell->val1_type;
    } else if (cell->expression_type == 1) {
        count = cell->val1_type + cell->val2_type;
    } else if (cell->expression_type == 2) {
        if (cell->val2_type == 1 && cell->op == 1) {
            count = cell->val1_type;
        } else {
            count = (cell_index_to_row(cell->val2) - cell_index_to_row(cell->val1) + 1) * (
                        cell_index_to_col(cell->val2) - cell_index_to_col(cell->val1) + 1);
        }
    }
    return count;
}

void clean_cells(const int start_cell) {
    clear_stack();
    stack_push(start_cell);
    while (!is_stack_empty()) {
        const int current_index = stack_top();
        Cell *current = get_cell(current_index);
        stack_pop();
        const pair eval = evaluate_cell(current);
        current->cell_state = eval.first ? 0 : 3;
        current->value = eval.second;
        if (current->dependants_type == ArrayForm) {
            int cell_index;
            for (int i = 0; i < current->dependants_array->size; i++) {
                cell_index = current->dependants_array->dependants_cells[i];
                const Cell *cell = get_cell(cell_index);
                if (cell->cell_state == 2) {
                    int flag = 1;
                    if (cell->expression_type == VALUE) {
                        if (cell->val1_type == CELL_REFERENCE) {
                            if (get_cell(cell->val1)->cell_state != 0 && get_cell(cell->val1)->cell_state != 3) flag = 0;
                        }
                    } else if (cell->expression_type == ARITHMETIC) {
                        if (cell->val1_type == CELL_REFERENCE) {
                            if (get_cell(cell->val1)->cell_state != 0 && get_cell(cell->val1)->cell_state != 3) flag = 0;
                        }
                        if (cell->val2_type == CELL_REFERENCE) {
                            if (get_cell(cell->val2)->cell_state != 0 && get_cell(cell->val2)->cell_state != 3) flag = 0;
                        }
                    } else {
                        if (cell->val2_type == 1 && cell->op == 1 && cell->val1_type == CELL_REFERENCE) {
                            if (get_cell(cell->val1)->cell_state != 0 && get_cell(cell->val1)->cell_state != 3) flag = 0;
                        } else {
                            const short start_row = cell_index_to_row(cell->val1);
                            const short start_col = cell_index_to_col(cell->val1);
                            const short end_row = cell_index_to_row(cell->val2);
                            const short end_col = cell_index_to_col(cell->val2);
                            for (short j = start_row; j <= end_row; j++) {
                                for (short k = start_col; k <= end_col; k++) {
                                    const Cell *dependency_cell = get_cell(rowcol_to_cell_index(j, k));
                                    if (dependency_cell->cell_state != 0 && dependency_cell->cell_state != 3) {
                                        flag = 0;
                                        break;
                                    }
                                }
                                if (flag == 0) {
                                    break;
                                }
                            }
                        }
                    }
                    if (flag) {
                        stack_push(cell_index);
                    }
                }
            }
        } else {
            SetIterator *iter = set_iterator_create(current->dependants_set);
            int cell_index;
            while ((cell_index = set_iterator_next(iter)) != -1) {
                const Cell *cell = get_cell(cell_index);
                if (cell->cell_state == 2) {
                    int flag = 1;
                    if (cell->expression_type == VALUE) {
                        if (cell->val1_type == CELL_REFERENCE) {
                            if (get_cell(cell->val1)->cell_state != 0 && get_cell(cell->val1)->cell_state != 3) flag = 0;
                        }
                    } else if (cell->expression_type == ARITHMETIC) {
                        if (cell->val1_type == CELL_REFERENCE) {
                            if (get_cell(cell->val1)->cell_state != 0 && get_cell(cell->val1)->cell_state != 3) flag = 0;
                        }
                        if (cell->val2_type == CELL_REFERENCE) {
                            if (get_cell(cell->val2)->cell_state != 0 && get_cell(cell->val2)->cell_state != 3) flag = 0;
                        }
                    } else {
                        if (cell->val2_type == 1 && cell->op == 1 && cell->val1_type == CELL_REFERENCE) {
                            if (get_cell(cell->val1)->cell_state != 0 && get_cell(cell->val1)->cell_state != 3) flag = 0;
                        } else {
                            const short start_row = cell_index_to_row(cell->val1);
                            const short start_col = cell_index_to_col(cell->val1);
                            const short end_row = cell_index_to_row(cell->val2);
                            const short end_col = cell_index_to_col(cell->val2);
                            for (short j = start_row; j <= end_row; j++) {
                                for (short k = start_col; k <= end_col; k++) {
                                    const Cell *dependency_cell = get_cell(rowcol_to_cell_index(j, k));
                                    if (dependency_cell->cell_state != 0 && dependency_cell->cell_state != 3) {
                                        flag = 0;
                                        break;
                                    }
                                }
                                if (flag == 0) {
                                    break;
                                }
                            }
                        }
                    }
                    if (flag) {
                        stack_push(cell_index);
                    }
                }
            }
            set_iterator_destroy(iter);
        }
    }
}

void clean_cell(Cell *cell);

int get_cell_value(const int cell_index) {
    return get_cell(cell_index)->value;
}

pair function_compute(const Cell *cell) {
    int ans = 0;
    pair ret;
    const int function_type = cell->val2_type * 4 + cell->op;
    if (function_type != SLEEP) {
        const short start_row = cell_index_to_row(cell->val1);
        const short start_col = cell_index_to_col(cell->val1);
        const short end_row = cell_index_to_row(cell->val2);
        const short end_col = cell_index_to_col(cell->val2);
        if (function_type == MIN) {
            const Cell *dep = get_cell(cell->val1);
            if (dep->cell_state == 3)
                goto zero_error_func;
            ans = dep->value;
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    dep = get_cell(rowcol_to_cell_index(i, j));
                    if (dep->cell_state == 3)
                        goto zero_error_func;
                    if (dep->value < ans) {
                        ans = dep->value;
                    }
                }
            }
        } else if (function_type == MAX) {
            const Cell *dep = get_cell(cell->val1);
            if (dep->cell_state == 3)
                goto zero_error_func;
            ans = dep->value;
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    dep = get_cell(rowcol_to_cell_index(i, j));
                    if (dep->cell_state == 3)
                        goto zero_error_func;
                    if (dep->value > ans) {
                        ans = dep->value;
                    }
                }
            }
        } else if (function_type == AVG) {
            ans = 0;
            int count = 0;
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    const Cell *dep = get_cell(rowcol_to_cell_index(i, j));
                    if (dep->cell_state == 3)
                        goto zero_error_func;
                    ans += dep->value;
                    count++;
                }
            }
            ans /= count;
        } else if (function_type == SUM) {
            ans = 0;
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    const Cell *dep = get_cell(rowcol_to_cell_index(i, j));
                    if (dep->cell_state == 3)
                        goto zero_error_func;
                    ans += dep->value;
                }
            }
        } else if (function_type == STDEV) {
            ans = 0;
            double variance = 0.0;
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    const Cell *dep = get_cell(rowcol_to_cell_index(i, j));
                    if (dep->cell_state == 3)
                        goto zero_error_func;
                    ans += dep->value;
                }
            }
            const int mean = ans / ((end_row - start_row + 1) * (end_col - start_col + 1));
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    const Cell *dep = get_cell(rowcol_to_cell_index(i, j));
                    if (dep->cell_state == 3)
                        goto zero_error_func;
                    variance += (dep->value - mean) * (dep->value - mean);
                }
            }
            variance /= (end_row - start_row + 1) * (end_col - start_col + 1);
            ans = (int) round(sqrt(variance));
            // float temp = 0;
            // float temp_sq = 0;
            // for (short i = start_row; i <= end_row; i++) {
            //     for (short j = start_col; j <= end_col; j++) {
            //         const Cell *dep = get_cell(rowcol_to_cell_index(i, j));
            //         if (dep->cell_state == ZERO_ERROR) goto zero_error_func;
            //         temp += dep->value;
            //     }
            // }
            // const int size = (end_row - start_row + 1) * (end_col - start_col + 1);
            // const float avg = temp / (float)size;
            //
            // for (short i = start_row; i <= end_row; i++) {
            //     for (short j = start_col; j <= end_col; j++) {
            //         temp_sq += ((float) get_raw_value(rowcol_to_cell_index(i, j)) - avg) * ((float) get_raw_value(rowcol_to_cell_index(i, j)) - avg);
            //     }
            // }
            //
            // ans = (int)sqrt(temp_sq / (double)size);
        }
    } else {
        if (cell->val1_type == INTEGER) {
            // sleep(cell->val1 > 0 ? cell->val1 : 0);
            ans = cell->val1;
        } else if (cell->val1_type == CELL_REFERENCE) {
            const Cell *dep = get_cell(cell->val1);
            if (dep->cell_state == 3)
                goto zero_error_func;
            ans = get_raw_value(cell->val1);
            // sleep(ans > 0 ? ans : 0);
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

pair evaluate_cell(const Cell *cell) {
    int eval = 0;
    if (cell->expression_type == VALUE) {
        if (cell->val1_type == INTEGER) {
            eval = cell->val1;
        } else if (cell->val1_type == CELL_REFERENCE) {
            const Cell *dependency = get_cell(cell->val1);
            eval = dependency->value;
            if (dependency->cell_state == 3)
                goto zero_error;
        } else {
            printf("Value holds Error State");
            exit(1);
        }
    } else if (cell->expression_type == ARITHMETIC) {
        int val1 = 0;
        int val2 = 0;
        if (cell->val1_type == INTEGER) {
            val1 = cell->val1;
        } else if (cell->val1_type == CELL_REFERENCE) {
            const Cell *dependency = get_cell(cell->val1);
            val1 = dependency->value;
            if (dependency->cell_state == 3)
                goto zero_error;
        } else {
            printf("Value holds Error State");
            exit(1);
        }

        if (cell->val2_type == INTEGER) {
            val2 = cell->val2;
        } else if (cell->val2_type == CELL_REFERENCE) {
            const Cell *dependency = get_cell(cell->val2);
            val2 = dependency->value;
            if (dependency->cell_state == 3)
                goto zero_error;
        } else {
            printf("Value holds Error State");
            exit(1);
        }

        if (cell->op == ADD) {
            eval = val1 + val2;
        } else if (cell->op == SUBTRACT) {
            eval = val1 - val2;
        } else if (cell->op == MULTIPLY) {
            eval = val1 * val2;
        } else if (cell->op == DIVIDE) {
            if (val2 == 0) {
                goto zero_error;
            }
            eval = val1 / val2;
        }
    } else if (cell->expression_type == FUNCTION) {
        return function_compute(cell);
    }
    return (pair){1, eval};
zero_error:
    eval = 0;
    return (pair){0, eval};
}

int handle_circular_connection(const int cell_index, const int prev_metadata, const int prev_val1,
                               const int prev_val2) {
    Cell *cell = get_cell(cell_index);
    if (!circular_check(cell_index)) {
        clear_debris();
        // delete current cell as a dependant from its new dependencies
        if (cell->expression_type == VALUE) {
            if (cell->val1_type == CELL_REFERENCE) {
                delete_dependant(cell->val1, cell_index);
            }
        } else if (cell->expression_type == ARITHMETIC) {
            if (cell->val1_type == CELL_REFERENCE) {
                delete_dependant(cell->val1, cell_index);
            }
            if (cell->val2_type == CELL_REFERENCE) {
                delete_dependant(cell->val2, cell_index);
            }
        } else {
            if (cell->val2_type == 1 && cell->op == 1 && cell->val1_type == CELL_REFERENCE) {
                delete_dependant(cell->val1, cell_index);
            } else {
                const short start_row = cell_index_to_row(cell->val1);
                const short start_col = cell_index_to_col(cell->val1);
                const short end_row = cell_index_to_row(cell->val2);
                const short end_col = cell_index_to_col(cell->val2);
                for (short i = start_row; i <= end_row; i++) {
                    for (short j = start_col; j <= end_col; j++) {
                        delete_dependant(rowcol_to_cell_index(i, j), cell_index);
                    }
                }
            }
        }
        // add current cell as a dependant to its old dependencies
        const int prev_expression_type = (255 & prev_metadata) >> 6;
        const int prev_val1_type = (63 & prev_metadata) >> 5;
        const int prev_val2_type = (31 & prev_metadata) >> 4;
        const int prev_op = (15 & prev_metadata) >> 2;
        if (prev_expression_type == VALUE) {
            if (prev_val1_type == CELL_REFERENCE) {
                add_dependant(prev_val1, cell_index);
            }
        } else if (prev_expression_type == ARITHMETIC) {
            if (prev_val1_type == CELL_REFERENCE) {
                add_dependant(prev_val1, cell_index);
            }
            if (prev_val2_type == CELL_REFERENCE) {
                add_dependant(prev_val2, cell_index);
            }
        } else if (prev_expression_type == FUNCTION) {
            if (prev_val2_type == 1 && prev_op == 1 && prev_val1_type == CELL_REFERENCE) {
                add_dependant(prev_val1, cell_index);
            } else {
                const short start_row = cell_index_to_row(prev_val1);
                const short start_col = cell_index_to_col(prev_val1);
                const short end_row = cell_index_to_row(prev_val2);
                const short end_col = cell_index_to_col(prev_val2);
                for (short i = start_row; i <= end_row; i++) {
                    for (short j = start_col; j <= end_col; j++) {
                        add_dependant(rowcol_to_cell_index(i, j), cell_index);
                    }
                }
            }
        }
        cell->expression_type = prev_expression_type;
        cell->val1_type = prev_val1_type;
        cell->val2_type = prev_val2_type;
        cell->op = prev_op;
        cell->val1 = prev_val1;
        cell->val2 = prev_val2;
        return 0;
    }
    clean_cells(cell_index);
    return 1;
}

int set_expression(const int cell_index, const int metadata, const int val1, const int val2) {
    Cell *cell = get_cell(cell_index);
    const int prev_metadata = cell->expression_type * 64 + cell->val1_type * 32 + cell->val2_type * 16 + cell->op * 4;
    const int prev_val1 = cell->val1;
    const int prev_val2 = cell->val2;
    const int expression_type = (255 & metadata) >> 6;
    const int val1_type = (63 & metadata) >> 5;
    const int val2_type = (31 & metadata) >> 4;
    const int op = (15 & metadata) >> 2;
    if (cell->expression_type == VALUE) {
        if (cell->val1_type == CELL_REFERENCE) {
            delete_dependant(cell->val1, cell_index);
        }
    } else if (cell->expression_type == ARITHMETIC) {
        if (cell->val1_type == CELL_REFERENCE) {
            delete_dependant(cell->val1, cell_index);
        }
        if (cell->val2_type == CELL_REFERENCE) {
            delete_dependant(cell->val2, cell_index);
        }
    } else if (cell->expression_type == FUNCTION) {
        if (cell->val2_type == 1 && cell->op == 1 && cell->val1_type == CELL_REFERENCE) {
            delete_dependant(cell->val1, cell_index);
        } else {
            const short start_row = cell_index_to_row(cell->val1);
            const short start_col = cell_index_to_col(cell->val1);
            const short end_row = cell_index_to_row(cell->val2);
            const short end_col = cell_index_to_col(cell->val2);
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    delete_dependant(rowcol_to_cell_index(i, j), cell_index);
                }
            }
        }
    }
    cell->expression_type = expression_type;
    cell->val1_type = val1_type;
    cell->val2_type = val2_type;
    cell->op = op;
    cell->val1 = val1;
    cell->val2 = val2;
    if (expression_type == VALUE) {
        if (val1_type == CELL_REFERENCE) {
            add_dependant(val1, cell_index);
        }
    } else if (expression_type == ARITHMETIC) {
        if (val1_type == CELL_REFERENCE) {
            add_dependant(val1, cell_index);
        }
        if (val2_type == CELL_REFERENCE) {
            add_dependant(val2, cell_index);
        }
    } else if (expression_type == FUNCTION) {
        if (val2_type == 1 && op == 1 && val1_type == CELL_REFERENCE) {
            add_dependant(val1, cell_index);
        } else {
            const short start_row = cell_index_to_row(val1);
            const short start_col = cell_index_to_col(val1);
            const short end_row = cell_index_to_row(val2);
            const short end_col = cell_index_to_col(val2);
            for (short i = start_row; i <= end_row; i++) {
                for (short j = start_col; j <= end_col; j++) {
                    add_dependant(rowcol_to_cell_index(i, j), cell_index);
                }
            }
        }
    }
    return handle_circular_connection(cell_index, prev_metadata, prev_val1, prev_val2);
}

int set_value_expression(const int cell_index, const Value value) {
    const int metadata = value.type == INTEGER ? 0 : 32;
    return set_expression(cell_index, metadata, value.value, 0);
}

int set_arithmetic_expression(const int cell_index, const Arithmetic arithmetic) {
    const int metadata = 64 + arithmetic.value1.type * 32 + arithmetic.value2.type * 16 + arithmetic.type * 4;
    return set_expression(cell_index, metadata, arithmetic.value1.value, arithmetic.value2.value);
}

int set_function_expression(const int cell_index, const Function function) {
    if (function.type == SLEEP) {
        const int metadata = 128 + function.value.type * 32 + function.type * 4;
        return set_expression(cell_index, metadata, function.value.value, 0);
    }
    const int metadata = 128 + function.type * 4;
    return set_expression(cell_index, metadata, function.range.start_index, function.range.end_index);
}
