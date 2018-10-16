#include "faasm.h"

/**
 * Passes some inputs onto other functions
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *funcNames[] = {"echo", "x2", "dummy"};

        for (uint8_t i = 0; i < 3; i++) {
            uint8_t funcData[3];
            funcData[0] = i;
            funcData[1] = i+1U;
            funcData[2] = i+2U;

            int dataLength = 3;

            memory->chainFunction(funcNames[i], funcData, dataLength);
        }

        return 0;
    }
}
