#include <stdio.h>
#include <stdlib.h>
#include "user_interface.h"

int main(int argc, char *argv[]) {
    // program will be run as ./prog n1 n2 where n1 and n2 are shorts
    if (argc != 3) {
        fprintf(stderr, "Usage: %s R C\n", argv[0]);
        return 1;
    }
    run_user_interface(atoi(argv[1]), atoi(argv[2]));
    return 0;
}