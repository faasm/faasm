#include "libA.h"
#include "sharedHeader.h"

#include <stdio.h>
#include <sys/mman.h>

SharedStruct sharedStructInstance = {
        "Shared struct",
        10,
        5,
        0   // Func pointer
};

int sharedFuncInstance(int a) {
    return a * a;
}

int multiply(int a, int b) {
    int result = a * b;
    printf("Multiplying %i and %i to get %i\n", a, b, result);

    return result;
}

int multiplyGlobal() {
    printf("Multiplying struct with name: %s\n", sharedStructInstance.name);
    return sharedStructInstance.alpha * sharedStructInstance.beta;
}

int checkStack() {
    int *heapStart = (int*) mmap(NULL, 1200 * sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    for(int i = 0; i < 1200; i++) {
        heapStart[i] = i;
    }

    int stackStart[100];
    for(int i = 0; i < 100; i++) {
        stackStart[i] = i;
    }

    int stackNext[50];
    for(int i = 0; i < 100; i++) {
        stackNext[i] = i;
    }

    // Check that the stack is below the heap
    int res = 0;
    long heapVsStack = &heapStart[0] - &stackStart[0];
    long stackGrowth = &stackStart[0] - &stackNext[0];
    if(heapVsStack < 0) {
        printf("Shared lib heap BELOW stack which is unexpected\n");
        res = 1;
    } else {
        printf("Shared lib heap above stack as expected\n");
    }
    
    // Check that the stack is growing downwards below the heap
    if(stackGrowth < 0) {
        printf("Shared lib stack growing UPWARDS which is unexpected\n");
        res = 1;
    } else {
        printf("Shared lib stack grows downwards as expected\n");
    }

    munmap(heapStart, 1200 * sizeof(int));
    return res;
}