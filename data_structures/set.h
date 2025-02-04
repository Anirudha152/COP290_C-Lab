#ifndef set_h
#define set_h
#include "../backend/primary_storage.h"
#include <stddef.h>

Set* set_create(void);
void set_destroy(Set *set);
int set_insert(Set *set, Cell *cell);
int set_remove(Set *set, short row, short col);
Cell* set_find(Set *set, short row, short col);
void set_clear(Set *set);
size_t set_size(const Set *set);

SetIterator* set_iterator_create(Set *set);
Cell* set_iterator_next(SetIterator *iterator);
void set_iterator_destroy(SetIterator *iterator);

#endif