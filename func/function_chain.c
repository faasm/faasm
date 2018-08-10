#include "faasm.h"

/**
 * Passes some inputs onto other functions
 */
EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    for (int i = 0; i < 20; i++) {
        char funcName[12];
        sprintf(funcName, "Function %i", i);

        uint8_t funcData[10];
        for (int j = 0; j < 10; j++) {
            funcData[j] = (uint8_t) ('a' + j);
        }

        chainFunction(memory, funcName, funcData, 10);
    }

    return 0;
}