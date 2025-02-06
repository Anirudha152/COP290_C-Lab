#ifndef STACK_H
#define STACK_H

#include "../backend/primary_storage.h"

void initialize_stack();
void destroy_stack();
Cell *stack_pop();
void stack_push(Cell *element);
Cell* stack_top();
int stack_size();
int is_stack_empty();
void clear_stack();

void initialize_stack_mem();
void destroy_stack_mem();
Memory stack_pop_mem();
void stack_push_mem(Memory element);
Memory stack_top_mem();
int stack_size_mem();
int is_stack_empty_mem();
void clear_stack_mem();

extern Stack cell_stack;
extern MemoryStack memory_stack;
#endif