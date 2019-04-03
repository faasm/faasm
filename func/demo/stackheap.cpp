#include <stdio.h>
#include <malloc.h>

/**
 * Demonstrates how stack and heap change
 */

void ptrDiff(int *a, int *b) {
    int intA = reinterpret_cast<int>(a);
    int intB = reinterpret_cast<int>(b);

    if (a > b) {
        printf("A (%i) > B (%i): A - B = %i\n", intA, intB, intA - intB);
    } else {
        printf("B (%i) > A (%i): B - A = %i\n", intB, intA, intB - intA);
    }
}

int main(int argc, char *argv[]) {
    // Create a couple of stack variables
    int stackA[20];
    int stackB[50];
    int stackC[100];

    int *heapA = (int *) malloc(20 * sizeof(int));
    auto heapB = new int[50];
    auto heapC = new int[100];

    ptrDiff(stackA, stackB);
    ptrDiff(stackB, stackC);
    ptrDiff(stackC, heapA);
    ptrDiff(heapA, heapB);
    ptrDiff(heapB, heapC);

    free(heapA);
    delete[] heapB;
    delete[] heapC;

    return 0;
}
