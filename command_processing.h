#ifndef COMMAND_PROCESSING_H
#define COMMAND_PROCESSING_H
#include "primary_storage.h"
typedef struct {
    char command[CMD_BUFFER_SIZE];
    double time_taken;
    int status;
    char error_msg[64];
} Command;
char *get_expression_string(const Expression *expr);
Command process_expression(char *input, const short row, const short col, const short viewport_row, const short viewport_col);
#endif
