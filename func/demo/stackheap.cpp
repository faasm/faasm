#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

/**
 * Demonstrates how stack and heap change
 */

void checkPtrGreater(int *a, int *b, const char *aName, const char *bName) {
    int diff = sizeof(int) * (a - b);

    if (diff < 0) {
        printf("Expected %s to be greater than %s but it wasn't (diff = %i)\n", aName, bName, diff);
        exit(1);
    }
}

void fillArray(int *stackPtr, int nElems) {
    for (int i = 0; i < nElems; i++) {
        stackPtr[i] = i;
    }
}

void stackSizeCheck() {
    // Want to create a big stack but not too big. We expect at least 1MB to be available.
    long minimumStack = 1 * 1024 * 1024;
    long nInts = (minimumStack / sizeof(int*));
    long sizeA = (nInts / 2) + 1;
    long sizeB = (nInts / 2) + 1;

    // Create arrays on the stack
    int stackA[sizeA];
    int stackB[sizeB];
    fillArray(stackA, sizeA);
    fillArray(stackB, sizeB);

    // Sense check order of the stack variables. The main stack should grow downwards but the contents of the
    // arrays should go upwards in order
    int *stackAEnd = &stackA[sizeA - 1];
    int *stackBEnd = &stackB[sizeB - 1];
    int *stackAStart = &stackA[0];
    int *stackBStart = &stackB[0];

    checkPtrGreater(stackAStart, stackBEnd, "stackAStart", "stackBEnd");
    checkPtrGreater(stackAEnd, stackAStart, "stackAEnd", "stackAStart");
    checkPtrGreater(stackBEnd, stackBStart, "stackBEnd", "stackBStart");

    // Check the size of the allocated stack region
    long diff = sizeof(int) * (stackAEnd - stackBStart);
    long expected = sizeof(int) * (sizeA + sizeB);
    if (diff < expected) {
        printf("Unexpectedly small amount of stack provisioned. Expected %li >= %li\n", diff, expected);
        exit(1);
    }

    if (diff < minimumStack) {
        printf("Provisioned stack is smaller than the minimum required stack of %li\n", minimumStack);
        exit(1);
    }

    printf("Stack space successfully provisioned %li (aimed for at least %li)\n", diff, expected);

    // Check they're both still intact (i.e. no corruption from overflows)
    for (int i = 0; i < sizeA; i++) {
        if (stackA[i] != i) {
            printf("Failed, stackA[%i] != %i\n", i, i);
            exit(1);
        }
    }

    for (int i = 0; i < sizeB; i++) {
        if (stackB[i] != i) {
            printf("Failed, stackB[%i] != %i\n", i, i);
            exit(1);
        }
    }

    printf("Stack check successful\n");
}

void stackCheck(int *argA, int *argB) {
    // Create new stack variables inside this method
    int stackA[10];
    int stackB[11];

    // Check the stack continues to grow downwards inside this function
    checkPtrGreater(argA, argB, "argA", "argB");
    checkPtrGreater(argB, stackA, "argB", "stackA");
    checkPtrGreater(stackA, stackB, "stackA", "stackB");
}

int main(int argc, char *argv[]) {
    // Create a couple of stack variables and check stack grows downwards
    int stackA[20];
    int stackB[50];
    int stackC[100];

    checkPtrGreater(&stackA[0], &stackB[0], "stackA", "stackB");
    checkPtrGreater(&stackB[0], &stackC[0], "stackB", "stackC");

    stackCheck(&stackA[0], &stackB[0]);

    stackSizeCheck();

    // Create some stuff on the heap, make sure it grows upwards
    int *heapA = (int *) malloc(20 * sizeof(int));
    auto heapB = new int[50];
    auto heapC = new int[100];

    checkPtrGreater(&heapC[0], &heapB[0], "heapC", "heapB");
    checkPtrGreater(&heapB[0], &heapA[0], "heapB", "heapA");

    // Check heap is above stack
    checkPtrGreater(&heapA[0], &stackA[0], "heapA", "stackA");

    free(heapA);
    delete[] heapB;
    delete[] heapC;

    return 0;
}
