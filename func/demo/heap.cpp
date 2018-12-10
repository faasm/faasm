#include "faasm/faasm.h"
#include <stdio.h>

/**
 * Creates an array on the heap. Used to check that heap memory is reclaimed
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        // Create array of ints
        int size = 1024*1024;
        auto heapArray = new int[size];
        for (int i = 0; i < size; i++) {
            heapArray[i] = i;
        }

        // Get pointer to first element
        int* heapPtr = &heapArray[0];
        printf("Heap ptr: %p\n", heapPtr);

        delete[] heapArray;
        return 0;
    }
}
