#include "faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        uint8_t output[5];
        for (int i = 0; i < 5; i++) {
            output[i] = (uint8_t) i;
        }

        memory->setOutput(output, 5);

        return 0;
    }
}