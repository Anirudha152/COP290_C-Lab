#ifndef cli_user_interface_h
#define cli_user_interface_h

typedef struct {
	short start_row;
	short start_col;
	short visible_rows;
	short visible_cols;
	int output_enabled;
} DisplayState;
extern DisplayState *state;
void run_user_interface();
#endif
