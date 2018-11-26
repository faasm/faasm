#include "faasm/faasm.h"

/**
 * Multiplies its array input by two
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        long inputSize = memory->getInputSize();
        auto inputBuffer = new uint8_t[inputSize];
        memory->getInput(inputBuffer, inputSize);

        auto output = new uint8_t[inputSize];

        for (int i = 0; i < inputSize; i++) {
            output[i] = inputBuffer[i] * 2;
        }

        memory->setOutput(output, inputSize);

        return 0;
    }
}
