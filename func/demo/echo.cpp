#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>

/**
 * Writes the input to the output
 */
FAASM_MAIN_FUNC() {
    long inputSize = faasmGetInputSize();

    // Handle empty input
    if (inputSize == 0) {
        const char *output = "Nothing to echo";
        auto bytesOutput = reinterpret_cast<const uint8_t *>(output);
        faasmSetOutput(bytesOutput, strlen(output));
        return 0;
    }

    auto inputBuffer = new uint8_t[inputSize];
    faasmGetInput(inputBuffer, inputSize);

    auto inputStr = reinterpret_cast<char *>(inputBuffer);
    printf("Echoing %s\n", inputStr);

    faasmSetOutput(inputBuffer, inputSize);
    return 0;
}
