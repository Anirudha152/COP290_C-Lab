#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "parsing/command_processing.h"
#include "backend/primary_storage.h"
#include "backend/compute_unit.h"
#include "gui/user_interface.h"

short tot_rows;
short tot_cols;
Stack cell_stack;
MemoryStack memory_stack;

int main(int argc, char *argv[]) {
    // program will be run as ./prog n1 n2 where n1 and n2 are shorts
    if (argc != 3) {
        fprintf(stderr, "Usage: %s R C\n", argv[0]);
        return 1;
    }
    tot_rows = (short)strtol(argv[1], NULL, 10);
    tot_cols = (short)strtol(argv[2], NULL, 10);
    initialize_storage();
    run_user_interface();
    destroy_storage();
    return 0;
}

// int main(const int argc, char *argv[]) {
//     if (argc != 3) {
//         fprintf(stderr, "Usage: %s R C\n", argv[0]);
//         return 1;
//     }
//     tot_rows = (short)strtol(argv[1], NULL, 10);
//     tot_cols = (short)strtol(argv[2], NULL, 10);
//     initialize_storage();
//     char inps[][256] = {"A1=B1", "B1=C1", "C1=5", "C3=SUM(A1:C1)", "C3=SUM(A1:C3)"};
//     for (int i = 0; i < 5; i++) {
//         Command com = process_expression(inps[i], 0, 0);
//         printf("%s ", com.command);
//         printf("%d ", com.status);
//         if (!com.status) printf("%s ", com.error_msg);
//         printf("%d\n", get_cell_value(2, 2));
//     }
//     destroy_storage();
//     return 0;
// }