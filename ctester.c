#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LENGTH 1024
#define MAX_CELLS 1000
#define CELL_ID_LENGTH 10
#define VALUE_LENGTH 20

// Structure to hold cell values
typedef struct {
    char cell[CELL_ID_LENGTH];
    char value[VALUE_LENGTH];
} CellValue;

// Function to extract values from output.txt
int extract_values_from_output(const char* file_path, CellValue* values, int max_values) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", file_path);
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    regex_t regex;
    regmatch_t matches[4];
    int count = 0;

    // Compile regex pattern for output.txt format
    if (regcomp(&regex, "([A-Za-z][0-9]+) : .*-->[ ]*Value: ([A-Za-z0-9\\-\\.!#]+), State: ([A-Za-z]+)", REG_EXTENDED)) {
        printf("Could not compile regex pattern\n");
        fclose(file);
        return 0;
    }

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL && count < max_values) {
        if (regexec(&regex, line, 4, matches, 0) == 0) {
            // Extract cell identifier
            int cell_len = matches[1].rm_eo - matches[1].rm_so;
            if (cell_len >= CELL_ID_LENGTH) cell_len = CELL_ID_LENGTH - 1;
            strncpy(values[count].cell, line + matches[1].rm_so, cell_len);
            values[count].cell[cell_len] = '\0';
            
            // Extract value
            int value_len = matches[2].rm_eo - matches[2].rm_so;
            if (value_len >= VALUE_LENGTH) value_len = VALUE_LENGTH - 1;
            strncpy(values[count].value, line + matches[2].rm_so, value_len);
            values[count].value[value_len] = '\0';
            
            // Extract state
            int state_len = matches[3].rm_eo - matches[3].rm_so;
            char state[20] = {0};
            if (state_len < 20) {
                strncpy(state, line + matches[3].rm_so, state_len);
                state[state_len] = '\0';
            }
            
            // If state is "Zero", replace value with "#DIV/0!"
            if (strcmp(state, "Zero") == 0) {
                strcpy(values[count].value, "#DIV/0!");
            }
            
            count++;
        }
    }

    regfree(&regex);
    fclose(file);
    return count;
}

// Function to extract values from model_output.txt
int extract_values_from_model(const char* file_path, CellValue* values, int max_values) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", file_path);
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    regex_t regex;
    regmatch_t matches[3];
    int count = 0;

    // Compile regex pattern for model_output.txt format
    if (regcomp(&regex, "([A-Za-z][0-9]+): ([A-Za-z0-9\\-\\.!#]+)", REG_EXTENDED)) {
        printf("Could not compile regex pattern\n");
        fclose(file);
        return 0;
    }

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL && count < max_values) {
        if (regexec(&regex, line, 3, matches, 0) == 0) {
            // Extract cell identifier
            int cell_len = matches[1].rm_eo - matches[1].rm_so;
            if (cell_len >= CELL_ID_LENGTH) cell_len = CELL_ID_LENGTH - 1;
            strncpy(values[count].cell, line + matches[1].rm_so, cell_len);
            values[count].cell[cell_len] = '\0';
            
            // Extract value
            int value_len = matches[2].rm_eo - matches[2].rm_so;
            if (value_len >= VALUE_LENGTH) value_len = VALUE_LENGTH - 1;
            strncpy(values[count].value, line + matches[2].rm_so, value_len);
            values[count].value[value_len] = '\0';
            
            count++;
        }
    }

    regfree(&regex);
    fclose(file);
    return count;
}

// Function to find a cell value in an array of CellValues
int find_cell(CellValue* values, int count, const char* cell, char* value) {
    for (int i = 0; i < count; i++) {
        if (strcmp(values[i].cell, cell) == 0) {
            strcpy(value, values[i].value);
            return 1;
        }
    }
    strcpy(value, "MISSING");
    return 0;
}

int main() {
    // File paths
    const char* output_file = "output.txt";
    const char* model_output_file = "model_output.txt";
    
    // Arrays to store cell values
    CellValue output_values[MAX_CELLS];
    CellValue model_values[MAX_CELLS];
    
    // Extract values from both files
    int output_count = extract_values_from_output(output_file, output_values, MAX_CELLS);
    int model_count = extract_values_from_model(model_output_file, model_values, MAX_CELLS);
    
    // Compare values
    int differences_found = 0;
    
    for (int i = 0; i < output_count; i++) {
        char model_value[VALUE_LENGTH];
        find_cell(model_values, model_count, output_values[i].cell, model_value);
        
        // Check for mismatch, with special handling for division by zero
        if (strcmp(output_values[i].value, model_value) != 0) {
            // Skip if one is "#DIV" and the other is "#DIV/0!"
            if (strcmp(model_value, "#DIV") == 0 && strcmp(output_values[i].value, "#DIV/0!") == 0) {
                continue;
            }
            
            printf("Mismatch at %s: output.txt has %s, but model_output.txt has %s\n", 
                   output_values[i].cell, output_values[i].value, model_value);
            differences_found = 1;
        }
    }
    
    // Print results
    if (!differences_found) {
        printf("✅ All values match!\n");
    }
    
    return 0;
}
