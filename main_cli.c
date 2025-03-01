#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "command_interface/user_interface.h"
#include "data_structures/set.h"

Stack cell_stack;
MemoryStack memory_stack;

short SCROLL_AMOUNT = 10;
short CMD_HISTORY_SIZE = 7;
short VIEWPORT_ROWS = 10;
short DEBUG_GUI = 0;
short GUI = 0;
short TOT_ROWS;
short TOT_COLS;
short LAZY_EVALUATION = 1;

void print_usage(const char *program_name) {
    printf("Usage: %s R C [options]\n", program_name);
    printf("Options:\n");
    printf("  -s, --scroll <n>               Lines to scroll in spreadsheet using WASD (default 10)\n");
    printf("  -v, --viewport <n>             Max number of rows/cols to display        (default 10)\n");
    printf("  -h, --help                     Show this help message\n");
    printf("%lu", sizeof(Value));
}

void parse_arguments(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        exit(1);
    }
    TOT_ROWS = (short) strtol(argv[1], NULL, 10);
    TOT_COLS = (short) strtol(argv[2], NULL, 10);
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        }
        if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scroll") == 0) && i + 1 < argc) SCROLL_AMOUNT = atoi(argv[++i]);
        else if ((strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--viewport") == 0) && i + 1 < argc) VIEWPORT_ROWS = atoi(argv[++i]) < TOT_ROWS ? atoi(argv[i]) : TOT_ROWS;
        else {
            printf("Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
	parse_arguments(argc, argv);
	initialize_storage();
	run_user_interface();
	destroy_storage();
	return 0;
}