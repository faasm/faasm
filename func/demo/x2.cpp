#include "faasm/faasm.h"

/**
 * Multiplies its array input by two
 */
int main(int argc, char* argv[])
{
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
