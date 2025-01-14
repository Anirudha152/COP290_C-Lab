#include <stdio.h>
#include "primary_storage.h"

int main(void) {
    initStorage(1000, 1000);
    setValue(0, 0, 10);
    setValue(0, 1, 20);
    setValue(0, 2, 30);
    
    for (int i=0;i<1000;i++) {
        for (int j=0;j<1000;j++) {
            addDependant(i,j,1,1);
            short row_arr[1]={1};
            short col_arr[1]={1};
            updateDependencies(row_arr, col_arr, 1, i, j);
        }
    }
    for (int i=0;i<1000;i++) {
        for (int j=0;j<1000;j++) {
            Cell *cell=getCell(i,j);
            if (cell->dependant_count!=1000) {
                printf("Error: %d %d\n", i, j);
            }
        }
    }
    return 0;

}