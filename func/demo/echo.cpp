#include "faasm/faasm.h"

#include <stdio.h>

/**
 * Writes the input to the output
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        long inputSize = memory->getInputSize();
        const uint8_t *input = memory->getInput();
        printf("MEMORY IN: %s \n", input);

        memory->setOutput(input, inputSize);
        return 0;
    }
}
