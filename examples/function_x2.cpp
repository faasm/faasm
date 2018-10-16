#include "faasm.h"

/**
 * Multiplies its array input by two
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        uint8_t output[MAX_INPUT_BYTES];

        for (int i = 0; i < MAX_INPUT_BYTES; i++) {
            output[i] = input[i] * 2;
        }

        memory->setOutput(output, MAX_INPUT_BYTES);

        return 0;
    }
}
