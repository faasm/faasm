#include "faasm.h"

/**
 * Passes some inputs onto other functions
 */
EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    for (int i = 0; i < 3; i++) {
        char funcName[12];
        sprintf(funcName, "Function %i", i);

        uint8_t funcData[] = {i, i + 1, i + 2};
        int dataLength = 3;

        chainFunction(memory, funcName, funcData, dataLength);
    }

    return 0;
}