#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include "../constants.h"
#include "../parsing/cell_indexing.h"
#include "../backend/compute_unit.h"
#include"../parsing/command_processing.h"
#include "user_interface.h"

DisplayState *state = NULL;
Command last_command;

void initialize_display() {
	state = malloc(sizeof(DisplayState));
	last_command = (Command) {
		.command = "",
		.time_taken = 0.0,
		.status = 0,
		.error_msg = ""
	};
	if (!state) {
		printf("Memory allocation failed\n");
		return;
	}
	state->start_row = 0;
	state->start_col = 0;
	state->visible_rows = VIEWPORT_ROWS > TOT_ROWS ? TOT_ROWS : VIEWPORT_ROWS;
	state->visible_cols = VIEWPORT_ROWS > TOT_COLS ? TOT_COLS : VIEWPORT_ROWS;
	state->output_enabled = 1;
}

void cleanup_display() {
	if (state) {
		free(state);
	}
}

void move_to(const short row, const short col) {
	state->start_row = max(min(row, TOT_ROWS - 1), 0);
	state->start_col = max(min(col, TOT_COLS - 1), 0);
	state->visible_rows = min(VIEWPORT_ROWS, TOT_ROWS - state->start_row);
	state->visible_cols = min(VIEWPORT_ROWS, TOT_COLS - state->start_col);
}

void handle_movement_command(const char cmd) {
	switch (cmd) {
		case 'w':
			if (state->start_row != 0) move_to(state->start_row - SCROLL_AMOUNT, state->start_col);
		break;
		case 's':
			if (state->start_row != TOT_ROWS - 1) move_to(state->start_row + SCROLL_AMOUNT, state->start_col);
		break;
		case 'a':
			if (state->start_col != 0) move_to(state->start_row, state->start_col - SCROLL_AMOUNT);
		break;
		case 'd':
			if (state->start_col != TOT_COLS - 1) move_to(state->start_row, state->start_col + SCROLL_AMOUNT);
		break;
		default:
			break;
	}

}

void draw_grid() {
	if (state->output_enabled) {
		printf("%-*s", DEFAULT_CELL_WIDTH, "");
		for (short j = 0; j < state->visible_cols; j++) {
			const short actual_col = j + state->start_col;
			char col_label[MAX_COL_LABEL];
			col_index_to_label(actual_col, col_label);
			printf("%-*s", DEFAULT_CELL_WIDTH, col_label);
		}
		for (short i = 0; i < state->visible_rows; i++) {
			const short actual_row = i + state->start_row;
			printf("\n%-*d", DEFAULT_CELL_WIDTH, actual_row + 1);
			for (short j = 0; j < state->visible_cols; j++) {
				const short actual_col = j + state->start_col;
				const int value = get_cell_value(rowcol_to_cell_index(actual_row, actual_col));
				const Cell* cell = get_cell(rowcol_to_cell_index(actual_row, actual_col));
				if (cell->cell_state == 3) {
					printf("%-*s", DEFAULT_CELL_WIDTH, "ERR");
				} else {
					printf("%-*d", DEFAULT_CELL_WIDTH, value);
				}
			}
		}
		printf("\n");
	}
}

int run() {
	printf("[%.1f] (%s) > ", last_command.time_taken, last_command.status ? "ok" : last_command.error_msg);
	char command[CMD_BUFFER_SIZE];
	char error_msg[64];
	fgets(command, CMD_BUFFER_SIZE, stdin);
	char *temp = command;
	while (*temp++ != '\n') {}
	*--temp = '\0';
	strcpy(last_command.command, command);
	struct timespec start, finish, delta;
	clock_gettime(CLOCK_REALTIME, &start);
	if (strlen(command) == 1 && strchr("wasd", command[0])) {
		handle_movement_command(command[0]);
		for (short i = 0; i < state->visible_rows; i++) {
			for (short j = 0; j < state->visible_cols; j++) {
				get_cell_value(rowcol_to_cell_index(i + state->start_row, j + state->start_col));
			}
		}
		last_command.status = 1;
	} else if (strlen(command) == 1 && strchr("q", command[0])) {
		return 0;
	} else if (strlen(command) == 0 && command[0] == '\0') {
		strcpy(error_msg, "Empty Command");
		goto error;
	} else if (!abs(strcmp(command, "disable_output"))) {
		state->output_enabled = 0;
		last_command.status = 1;
		last_command.time_taken = 0.0;
	} else if (!abs(strcmp(command, "enable_output"))) {
		state->output_enabled = 1;
		last_command.status = 1;
		last_command.time_taken = 0.0;
	} else if (strncmp(command, "scroll_to", 9) == 0) {
		if (command[9] != ' ') {
			strcpy(error_msg, "Invalid scroll argument");
			goto error;
		}
		short row, col;
		const int resp = parse_cell_reference(command + 10, &row, &col);
		if (!resp) {
			strcpy(error_msg, "Invalid scroll argument");
			goto error;
		}
		if (row >= 0 && row < TOT_ROWS && col >= 0 && col < TOT_COLS) {
			move_to(row, col);
		} else {
			strcpy(error_msg, "Cell Reference Out of Bounds");
			goto error;
		}
		last_command.status = 1;
		last_command.time_taken = 0.0;
	} else {
		last_command = process_expression(command);
	}
	draw_grid();
	clock_gettime(CLOCK_REALTIME, &finish);
	last_command.time_taken = sub_timespec(start, finish, &delta);
	return 1;
error:
	draw_grid();
	last_command.status = 0;
	last_command.time_taken = 0.0;
	strcpy(last_command.error_msg, error_msg);
	strcpy(last_command.command, command);
	return 1;
}


void run_user_interface() {
	initialize_display();
	if (!state) return;
	draw_grid();
	last_command = (Command) {"",0.0,1,""};
	while (run()) {}
	cleanup_display();
}
