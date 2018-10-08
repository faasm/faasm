#include "faasm.h"

/**
 * Passes some inputs onto other functions
 */
int exec(struct FaasmMemory *memory) {
    char *funcNames[] = {"echo", "x2", "dummy"};

    for (int i = 0; i < 3; i++) {
        uint8_t funcData[] = {i, i + 1, i + 2};
        int dataLength = 3;

        chainFunction(memory, funcNames[i], funcData, dataLength);
    }

    return 0;
}