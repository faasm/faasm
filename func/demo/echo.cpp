#if AWS_LAMBDA == 1
// Handle AWS context with different import
#include "lambda/lambda.h"
#else

#include "faasm/faasm.h"
#endif

/**
 * Writes the input to the output
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        long inputSize = memory->getInputSize();
        auto inputBuffer = new uint8_t[inputSize];
        memory->getInput(inputBuffer, inputSize);

        memory->setOutput(inputBuffer, inputSize);
        return 0;
    }
}
