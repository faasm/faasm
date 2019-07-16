#include <faasm/faasm.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

FAASM_MAIN_FUNC() {
    long inputSize = faasmGetInputSize();

    // Handle empty input
    if (inputSize == 0) {
        printf("Nothing to echo\n");
        const char *output = "Nothing to echo";
        uint8_t *bytesOutput = (uint8_t *) output;

        faasmSetOutput(bytesOutput, strlen(output));
        return 0;
    }

    uint8_t *inputBuffer = malloc(inputSize * sizeof(uint8_t));
    faasmGetInput(inputBuffer, inputSize);

    char *inputStr = (char *) inputBuffer;
    printf("Echoing %s\n", inputStr);

    faasmSetOutput(inputBuffer, inputSize);

    free(inputBuffer);

    return 0;
}
