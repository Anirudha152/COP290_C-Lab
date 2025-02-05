# COP290-C_LAB

## Team Members
1. Anirudha Saraf (2023CS10806)
2. Nishchay Patil (2023CS10269)
3. Ayush Kumar Singh (2023CS10477)

## Assignment Description
This repository contains the code for the C Lab Assignment for COP290 course. The assignment is to implement a simplified spreadsheet program in C similar to Excel. The program stores the matrix in memory with a scrollable viewport on display. It will support the below specified functions along with lazy evaluation and storing the data on the heap to maximise the efficiency and scalability of the program.
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
- The next `n` lines will contain expressions to set values to cells
- The following `m` lines will contain queries to get the value of a cell
  - The queries will be of the form: `<CELL>` or `scroll_to <CELL>`
