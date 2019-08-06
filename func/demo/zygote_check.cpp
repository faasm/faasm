#include "faasm/faasm.h"
#include <stdio.h>

int globalA = 0;
int globalB = 2;

FAASM_ZYGOTE() {
    // Perform operation that will show if it's done more than once
    globalA += 5;
    globalB += 100;
}

/**
 * Checks zygotes are working
 */
FAASM_MAIN_FUNC() {
    if(globalA == 5 && globalB == 102) {
        printf("Zygote function working");
        return 0;
    } else {
        printf("Zygote function not working as expected");
        return 1;
    }
}
