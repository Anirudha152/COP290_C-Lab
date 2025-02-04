#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "cli/user_interface.h"
#include "data_structures/set.h"

short tot_rows;
short tot_cols;
Stack cell_stack;
MemoryStack memory_stack;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s R C\n", argv[0]);
		return 1;
	}
	if (GUI) {
		fprintf(stderr, "Set GUI to 0 in constants.h");
		return 1;
	}
	tot_rows = (short)strtol(argv[1], NULL, 10);
	tot_cols = (short)strtol(argv[2], NULL, 10);
	initialize_storage();
	run_user_interface();
	destroy_storage();
	return 0;
}