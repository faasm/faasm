#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>

/**
 * Writes the input to the output
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        long inputSize = memory->getInputSize();

        // Handle empty input
        if (inputSize == 0) {
            const char *output = "Nothing to echo";
            auto bytesOutput = reinterpret_cast<const uint8_t *>(output);
            memory->setOutput(bytesOutput, strlen(output));
            return 0;
        }

        auto inputBuffer = new uint8_t[inputSize];
        memory->getInput(inputBuffer, inputSize);

        auto inputStr = reinterpret_cast<char *>(inputBuffer);
        printf("Echoing %s\n", inputStr);

        memory->setOutput(inputBuffer, inputSize);
        return 0;
    }
}
