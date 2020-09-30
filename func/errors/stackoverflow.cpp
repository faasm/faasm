#include <stdio.h>

/**
 * Causes a SO
 */

int main(int argc, char* argv[])
{
    // Filling up a big array on the stack will cause a stack overflow
    int lenA = 16000;
    int stackA[lenA];
    for (int i = 0; i < lenA; i++) {
        stackA[i] = i;
    }

    printf("Stack top: %i\n", &stackA[0]);

    // Shouldn't reach this point
    return 0;
}
