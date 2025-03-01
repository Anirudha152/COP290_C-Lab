#ifndef COMMAND_PROCESSING_H
#define COMMAND_PROCESSING_H
// #include "../backend/primary_storage.h"
typedef struct {
    char command[CMD_BUFFER_SIZE];
    double time_taken;
    int status;
    char error_msg[64];
} Command;
char *get_expression_string(int cell_index);
Command process_expression(const char *command);
#endif
