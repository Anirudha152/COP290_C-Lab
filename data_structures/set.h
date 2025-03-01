#ifndef set_h
#define set_h
#include "../backend/primary_storage.h"
#include <stddef.h>

Set* set_create(void);
void set_destroy(Set *set);
int set_insert(Set *set, int cell_index);
int set_remove(Set *set, int cell_index);
int set_find(Set *set, int cell_index);
void set_clear(Set *set);
size_t set_size(const Set *set);

SetIterator* set_iterator_create(Set *set);
int set_iterator_next(SetIterator *iterator);
void set_iterator_destroy(SetIterator *iterator);

#endif