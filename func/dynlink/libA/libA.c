#include "libA.h"
#include "sharedHeader.h"

#include <stdio.h>
#include <sys/mman.h>

SharedStruct sharedStructInstance = {
    "Shared struct",
    10,
    5,
    0 // Func pointer
};

int sharedFuncInstance(int a)
{
    return a * a;
}

int multiply(int a, int b)
{
    int result = a * b;
    printf("Multiplying %i and %i to get %i\n", a, b, result);

    return result;
}

int multiplyGlobal()
{
    printf("Multiplying struct with name: %s\n", sharedStructInstance.name);
    return sharedStructInstance.alpha * sharedStructInstance.beta;
}

int checkStack()
{
    // Provision space on both the heap and stack
    int heapSize = 1200;
    int* heapStart = (int*)mmap(NULL,
                                heapSize * sizeof(int),
                                PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS,
                                -1,
                                0);
    for (int i = 0; i < heapSize; i++) {
        heapStart[i] = 10 * i;
    }

    // Make the stack space big to ensure we're getting more than 1MB
    int minimumStack = 1024 * 1024;
    int arraySize = minimumStack / sizeof(int);
    int stackStart[arraySize];
    for (int i = 0; i < arraySize; i++) {
        stackStart[i] = 2 * i;
    }

    // Add another array "above" this on the stack
    int arraySizeB = 50;
    int stackNext[arraySizeB];
    for (int i = 0; i < arraySizeB; i++) {
        stackNext[i] = 20 * i;
    }

    // Check everything is still intact (i.e. no stack overflows)
    for (int i = 0; i < heapSize; i++) {
        if (heapStart[i] != 10 * i) {
            printf("Heap corrupted at %i\n", i);
            return 1;
        }
    }

    for (int i = 0; i < arraySize; i++) {
        if (stackStart[i] != 2 * i) {
            printf("First stack array corrupted at %i\n", i);
            return 1;
        }
    }

    for (int i = 0; i < arraySizeB; i++) {
        if (stackNext[i] != 20 * i) {
            printf("Second stack array corrupted at %i\n", i);
            return 1;
        }
    }

    // Check that the stack is below the heap
    int res = 0;
    long heapVsStack = &heapStart[0] - &stackStart[0];
    if (heapVsStack < 0) {
        printf("Shared lib heap BELOW stack which is unexpected\n");
        res = 1;
    } else {
        printf("Shared lib heap above stack as expected\n");
    }

    // Check that the stack is growing downwards below the heap
    long stackGrowth = &stackStart[0] - &stackNext[0];
    if (stackGrowth < 0) {
        printf("Shared lib stack growing UPWARDS which is unexpected\n");
        res = 1;
    } else {
        printf("Shared lib stack grows downwards as expected\n");
    }

    printf("All stack/ heap checks completed\n");

    munmap(heapStart, heapSize * sizeof(int));
    return res;
}
