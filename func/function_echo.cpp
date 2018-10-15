#include "faasm.h"

/**
 * Writes the input to the output
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        uint8_t *input = memory->getInput();
        printf("MEMORY IN: %s \n", input);

        memory->setOutput(input, MAX_OUTPUT_BYTES);
        return 0;
    }
}
