#include "faasm/faasm.h"

/**
 * Multiplies its array input by two
 */
FAASM_MAIN_FUNC() {
    long inputSize = faasmGetInputSize();
    auto inputBuffer = new uint8_t[inputSize];
    faasmGetInput(inputBuffer, inputSize);

    auto output = new uint8_t[inputSize];

    for (int i = 0; i < inputSize; i++) {
        output[i] = inputBuffer[i] * 2;
    }

    faasmSetOutput(output, inputSize);

    return 0;
}
