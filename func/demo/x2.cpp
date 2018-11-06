#include "faasm/faasm.h"

/**
 * Multiplies its array input by two
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        uint8_t output[100];

        for (int i = 0; i < 100; i++) {
            output[i] = input[i] * 2;
        }

        memory->setOutput(output, 100);

        return 0;
    }
}
