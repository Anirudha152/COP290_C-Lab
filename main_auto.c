#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "backend/compute_unit.h"
#include "parsing/cell_indexing.h"
#include "parsing/command_processing.h"

short tot_rows;
short tot_cols;
Stack cell_stack;
MemoryStack memory_stack;

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s R C\n", argv[0]);
        return 1;
    }
    if (!GUI) {
        fprintf(stderr, "Set GUI to 1 in constants.h");
        return 1;
    }
    tot_rows = (short)strtol(argv[1], NULL, 10);
    tot_cols = (short)strtol(argv[2], NULL, 10);
    int t;
    scanf("%d", &t);
    initialize_storage();
    while (t--) {
        int n, m;
        scanf("%d %d\n", &n, &m);
        char inps[n][CMD_BUFFER_SIZE];
        for (int i=0; i<n; i++) {
            char inp[CMD_BUFFER_SIZE];
            fgets(inp, CMD_BUFFER_SIZE, stdin);
            char *temp = inp;
            while (*temp++ != '\n') {}
            *--temp = '\0';
            strcpy(inps[i], inp);
        }
        short vrow = 0;
        short vcol = 0;
        for (int i = 0; i < n; i++) {
            if (strncmp(inps[i], "scroll_to", 9) == 0) {
                short row, col;
                int pos = parse_cell_reference(inps[i] + 10, &row, &col);
                if (!pos) continue;
                vrow = row;
                vcol = col;
                struct timespec start, finish, delta;
                clock_gettime(CLOCK_REALTIME, &start);
                for (short i = vrow; i < vrow + VIEWPORT_ROWS; i++) {
                    if (i >= tot_rows) break;
                    for (short j = vcol; j < vcol + VIEWPORT_ROWS; j++) {
                        if (j >= tot_cols) break;
                        get_cell_value(i, j);
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
            Command com = process_expression(inps[i], vrow, vcol);
            if (com.status) {
                strcpy(com.error_msg, "ok");
            }
            printf("[%d] > %s --> (%s) [%.3fs]\n", i + 1, com.command, com.error_msg, com.time_taken);
        }
        for (int i=0; i<m; i++) {
            char reference[CMD_BUFFER_SIZE];
            fgets(reference, CMD_BUFFER_SIZE, stdin);
            char *temp = reference;
            while (*temp++ != '\n') {}
            *--temp = '\0';
            short row, col;
            int pos = parse_cell_reference(reference, &row, &col);
            if (!pos) continue;
            Cell* cell = get_cell(row, col);
            char expression[CMD_BUFFER_SIZE];
            strcpy(expression, get_expression_string(&cell->formula));
            printf("%s : %s --> %d, %d\n", reference, expression, cell->value, cell->state);
        }
    }
    destroy_storage();
    return 0;
}
