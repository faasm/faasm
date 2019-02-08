#include "faasm/faasm.h"

#include <stdio.h>

/**
 * Writes the input to the output
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        long inputSize = memory->getInputSize();
        auto inputBuffer = new uint8_t[inputSize];
        memory->getInput(inputBuffer, inputSize);

        auto inputStr = reinterpret_cast<char*>(inputBuffer);
        printf("Echoing %s\n", inputStr);

        memory->setOutput(inputBuffer, inputSize);
        return 0;
    }
}
