#include <stdlib.h>
#include <stdio.h>
#include "../backend/primary_storage.h"
#include "stack.h"
#define DEFAULT_SIZE 4

void initialize_stack() {
    cell_stack.elements = malloc(sizeof(Cell*) * DEFAULT_SIZE);
    if (cell_stack.elements == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    cell_stack.no_of_elements = 0;
    cell_stack.dynamic_size = DEFAULT_SIZE;
}

void destroy_stack() {
    free(cell_stack.elements);
}

void shrink_capacity() {
    cell_stack.dynamic_size /= 2;
    Cell **temp_stack = malloc(sizeof(Cell*) * cell_stack.dynamic_size);
    if (temp_stack == NULL) {
        printf("Stack Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < cell_stack.no_of_elements; i++) {
        temp_stack[i] = cell_stack.elements[i];
    }
    free(cell_stack.elements);
    cell_stack.elements = temp_stack;
}

void double_capacity() {
    cell_stack.dynamic_size *= 2;
    cell_stack.elements = realloc(cell_stack.elements, sizeof(Cell*) * cell_stack.dynamic_size);
    if (cell_stack.elements == NULL) {
        printf("Stack Memory allocation failed\n");
        exit(1);
    }
}

Cell *stack_pop() {
    if (cell_stack.no_of_elements < cell_stack.dynamic_size / 2 && cell_stack.no_of_elements > DEFAULT_SIZE) {
        shrink_capacity();
    }
    cell_stack.no_of_elements -= 1;
    return cell_stack.elements[cell_stack.no_of_elements];
}

void stack_push(Cell *element) {
    if (cell_stack.elements == NULL) {
        printf("Stack Memory allocation failed\n");
        exit(1);
    }
    if (cell_stack.no_of_elements == cell_stack.dynamic_size) {
        double_capacity();
    }
    cell_stack.elements[cell_stack.no_of_elements] = element;
    cell_stack.no_of_elements += 1;
}

Cell* stack_top() {
    return cell_stack.elements[cell_stack.no_of_elements - 1];
}

int stack_size() {
    return cell_stack.no_of_elements;
}

int is_stack_empty() {
    return cell_stack.no_of_elements == 0;
}

void clear_stack() {
    cell_stack.no_of_elements = 0;
}

void initialize_stack_mem() {
    memory_stack.elements = malloc(sizeof(Memory) * DEFAULT_SIZE);
    if (memory_stack.elements == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    memory_stack.no_of_elements = 0;
    memory_stack.dynamic_size = DEFAULT_SIZE;
}

void destroy_stack_mem() {
    free(memory_stack.elements);
}

void shrink_capacity_mem() {
    memory_stack.dynamic_size /= 2;
    Memory* temp_stack = malloc(sizeof(Memory) * memory_stack.dynamic_size);
    if (temp_stack == NULL) {
        printf("Stack Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < memory_stack.no_of_elements; i++) {
        temp_stack[i] = memory_stack.elements[i];
    }
    free(memory_stack.elements);
    memory_stack.elements = temp_stack;
}

void double_capacity_mem() {
    memory_stack.dynamic_size *= 2;
    memory_stack.elements = realloc(memory_stack.elements, sizeof(Memory) * memory_stack.dynamic_size);
    if (memory_stack.elements == NULL) {
        printf("Stack Memory allocation failed\n");
        exit(1);
    }
}

Memory stack_pop_mem() {
    if (memory_stack.no_of_elements < memory_stack.dynamic_size / 2 && memory_stack.no_of_elements > DEFAULT_SIZE) {
        shrink_capacity_mem();
    }
    memory_stack.no_of_elements -= 1;
    return memory_stack.elements[memory_stack.no_of_elements];
}

void stack_push_mem(Memory element) {
    if (memory_stack.elements == NULL) {
        printf("Stack Memory allocation failed\n");
        exit(1);
    }
    if (memory_stack.no_of_elements == memory_stack.dynamic_size) {
        double_capacity_mem();
    }
    memory_stack.elements[memory_stack.no_of_elements] = element;
    memory_stack.no_of_elements += 1;
}

Memory stack_top_mem() {
    return memory_stack.elements[memory_stack.no_of_elements - 1];
}

int stack_size_mem() {
    return memory_stack.no_of_elements;
}

int is_stack_empty_mem() {
    return memory_stack.no_of_elements == 0;
}

void clear_stack_mem() {
    memory_stack.no_of_elements = 0;
}