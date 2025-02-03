#ifndef CELL_INDEXING_H
#define CELL_INDEXING_H
#include "../backend/primary_storage.h"
#include <stdbool.h>
bool cellWithinExpression(const Expression* expr, const short row, const short col);
short col_label_to_index(const char *col);
void col_index_to_label(short col, char *buffer);
int parse_cell_reference(const char *ref, short *row, short *col);
#endif
