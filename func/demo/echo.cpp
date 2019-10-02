#include "faasm/faasm.h"
#include "faasm/input.h"

#include <stdio.h>
#include <string.h>

/**
 * Writes the input to the output
 */
FAASM_MAIN_FUNC() {
    const char* inputStr = faasm::getStringInput("");
    size_t inputLen = strlen(inputStr);

    // Handle empty input
    if (inputLen == 0) {
        const char *output = "Nothing to echo";
        auto bytesOutput = reinterpret_cast<const uint8_t *>(output);
        faasmSetOutput(bytesOutput, strlen(output));
        return 0;
    }

    printf("Echoing %s\n", inputStr);

    faasmSetOutput(reinterpret_cast<const uint8_t *>(inputStr), inputLen);

    return 0;
}
