#include "faasm.h"

/**
 * Passes some inputs onto other functions
 */
int exec(struct FaasmMemory *memory) {
    printf("Chain names: %p\n", memory->chainFunctions);
    printf("Chain inputs: %p\n", memory->chainInputs);

    for (int i = 0; i < 3; i++) {
        char funcName[12];
        sprintf(funcName, "Function %i", i);

        uint8_t funcData[] = {i, i + 1, i + 2};
        int dataLength = 3;

        chainFunction(memory, funcName, funcData, dataLength);
    }

    return 0;
}