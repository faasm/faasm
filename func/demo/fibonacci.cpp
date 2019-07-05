#include "faasm/faasm.h"
#include <string>

/**
 * Deliberately inefficient fibonacci calculation for testing CPU hogs
 */
int doFib(int n) {
    if (n == 0) {
        return 0;
    } else if (n == 1) {
        return 1;
    } else {
        return doFib(n - 1) + doFib(n - 2);
    }
}

FAASM_MAIN_FUNC() {
    long inputSize = sizeof(int);

    uint8_t inputBuffer[inputSize];
    faasmGetInput(inputBuffer, inputSize);
    char *fibNumStr = reinterpret_cast<char *>(inputBuffer);

    int fibNum = std::stoi(fibNumStr);
    int result = doFib(fibNum);

    std::string output = "Fibonacci " + std::to_string(fibNum) + " = " + std::to_string(result);
    const uint8_t *outputBuffer;
    outputBuffer = reinterpret_cast<const uint8_t *>(output.c_str());
    faasmSetOutput(outputBuffer, output.size());

    return 0;
}