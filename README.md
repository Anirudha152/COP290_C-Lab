# COP290 C Lab

**Authors:** Anirudha Saraf, Ayush Kumar Singh, Nishchay Patil

## Assignment Description
The assignment is to implement a simplified spreadsheet program in C similar to Excel. The program stores the matrix in memory with a scrollable viewport on display. It will support the below specified functions along with lazy evaluation and storing the data on the heap to maximise the efficiency and scalability of the program.

The purpose of this assignment is to explore various design practices associated with C Programming as well as learning effective and safe memory management. It is significantly faster than excel at running heavily nested expressions, with long chains of cell dependencies consisting of simple expressions.

The expressions that each cell can contain are:
1. Value
    1. Constant
    2. Reference to another cell
2. Arithmetic Operations
    1. Addition
    2. Subtraction
    3. Multiplication
    4. Division
3. Mathematical Functions
    1. Sum
    2. Average
    3. Stdev
    4. Min
    5. Max
    6. Sleep

## Usage
The program is split into 3 runtypes
a) GUI (based on ncurses)
b) CLI (default)
c) Test (for testing purposes)

### GUI
To run the program in GUI mode, run the following command:
```bash
make gui
./sheet R C [options]
```
where R is the number of rows and C is the number of columns in the spreadsheet.
**R and C should be greater than 10 here**

GUI has 2 modes: **Interactive** and **Command Based** modes.
Interactive Mode:
- Use wasd to move the cursor, hold shift to move the spreadsheet, 
- Press enter to edit a cell's value, 
- Press backspace to set a cell's value to 0
- Use + and - to zoom the spreadsheet in and out
- Press q to quit the program
- Press tab to switch between modes

Command Based:
- Refer to CLI, basically the same in terms of functionality

### CLI
To run the program in CLI mode, run the following command:
```bash
make
./sheet R C [options]
```
where R is the number of rows and C is the number of columns in the spreadsheet.

CLI has only 1 mode: **Command Based** mode.
- Use `wasd` to move the spreadsheet
- Use expressions to edit cells
- Use `disable_output` to disable output
- Use `enable_output` to enable output
- Use `scroll_to <CELL>` to scroll to a cell
- Use `q` to exit program

### Test
To run the program in Test mode, run the following command:
```bash
make test
./sheet R C [options]
```
where R is the number of rows and C is the number of columns in the spreadsheet.

Test mode takes input from stdin similar to a codeforces input
- The first line should contain the number of test cases
- For each test case, the first line will contain `n` and `m`
  - `n` is the number of edits
  - `m` is the number of queries
- The next `n` lines will contain expressions to set values to cells, or the `scroll_to <CELL>` command which will change the virtual viewport of the program, for lazy evaluation
- The following `m` lines will contain queries in the form of `<CELL>` to get the **raw** value of a cell. If lazy evaluation is enabled, this may return a dirty cell with an unprocessed value.

## Disclaimer 
This code was developed as part of an academic assignment for the COP290 course at IIT Delhi. It is intended for educational purposes and was completed in March 2025, it is no longer being actively updated or maintained, please reach out to me over email or linkedin for any queries. Please cite appropriately if used in research or projects. Please refer to the included report for more details
