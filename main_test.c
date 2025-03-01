#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "backend/compute_unit.h"
#include "parsing/cell_indexing.h"
#include "parsing/command_processing.h"

Stack cell_stack;
MemoryStack memory_stack;

short SCROLL_AMOUNT = 10;
short CMD_HISTORY_SIZE = 7;
short VIEWPORT_ROWS = 10;
short DEBUG_GUI = 0;
short GUI = 1;
short TOT_ROWS;
short TOT_COLS;
short LAZY_EVALUATION = 1;

void print_usage(const char *program_name) {
    printf("Usage: %s R C [options]\n", program_name);
}

void parse_arguments(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        exit(1);
    }
    TOT_ROWS = (short) strtol(argv[1], NULL, 10);
    TOT_COLS = (short) strtol(argv[2], NULL, 10);
    VIEWPORT_ROWS = 10;
}

int main(const int argc, char *argv[]) {
    parse_arguments(argc, argv);
    int t;
    scanf("%d", &t);
    initialize_storage();
    while (t--) {
        int n, m;
        scanf("%d %d\n", &n, &m);
        char inps[n][CMD_BUFFER_SIZE];
        short vrow = 0;
        short vcol = 0;
        for (int i = 0; i < n; i++) {
            char inp[CMD_BUFFER_SIZE];
            fgets(inp, CMD_BUFFER_SIZE, stdin);
            char *temp = inp;
            while (*temp++ != '\n') {
            }
            *--temp = '\0';
            strcpy(inps[i], inp);
            if (strncmp(inps[i], "scroll_to", 9) == 0) {
                short row, col;
                const int pos = parse_cell_reference(inps[i] + 10, &row, &col);
                if (!pos)
                    continue;
                vrow = row;
                vcol = col;
                struct timespec start, finish, delta;
                clock_gettime(CLOCK_REALTIME, &start);
                for (short i_ = vrow; i_ < vrow + VIEWPORT_ROWS; i_++) {
                    if (i_ >= TOT_ROWS)
                        break;
                    for (short j_ = vcol; j_ < vcol + VIEWPORT_ROWS; j_++) {
                        if (j_ >= TOT_COLS)
                            break;
                        get_cell_value(rowcol_to_cell_index(i_, j_));
                    }
                }
                clock_gettime(CLOCK_REALTIME, &finish);
                Command com;
                strcpy(com.command, inps[i]);
                com.status = 1;
                com.time_taken = sub_timespec(start, finish, &delta);
                strcpy(com.error_msg, "ok");
                printf("[%d] > %s --> (%s) [%.3fs]\n", i + 1, com.command, com.error_msg, com.time_taken);
                continue;
            }
            Command com = process_expression(inps[i]);
            if (com.status) {
                strcpy(com.error_msg, "ok");
            }
            printf("[%d] > %s --> (%s) [%.3fs]\n", i + 1, com.command, com.error_msg, com.time_taken);
        }
        for (int i = 0; i < m; i++) {
            char reference[CMD_BUFFER_SIZE];
            fgets(reference, CMD_BUFFER_SIZE, stdin);
            char *temp = reference;
            while (*temp++ != '\n') {
            }
            *--temp = '\0';
            short row, col;
            const int pos = parse_cell_reference(reference, &row, &col);
            if (!pos)
                continue;
            const Cell *cell = get_cell(rowcol_to_cell_index(row, col));
            char expression[CMD_BUFFER_SIZE];
            strcpy(expression, get_expression_string(rowcol_to_cell_index(row, col)));
            printf("%s : %s --> Value: %d, State: %s\n", reference, expression, cell->value, cell->cell_state == CLEAN
                ? "Clean"
                : cell->cell_state == DFS_IN_PROGRESS
                     ? "DFS In Progress"
                     : cell->cell_state == CIRCULAR_CHECKED
                           ? "Circular Checked"
                           : "Zero Error");
        }
    }
    destroy_storage();
    return 0;
}
