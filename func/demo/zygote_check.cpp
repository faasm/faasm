#include "faasm/faasm.h"
#include <stdio.h>

int globalA = 0;
int globalB = 2;

FAASM_ZYGOTE()
{
    globalA += 5;
    globalB += 100;

    return 0;
}

/**
 * Checks zygotes are working
 */
int main(int argc, char* argv[])
{
    if (globalA == 5 && globalB == 102) {
        printf("Zygote function working\n");
        return 0;
    } else {
        printf("Zygote function not working as expected\n");
        return 1;
    }
}
