#include "faasm/faasm.h"
#include <string>
#include <faasm/input.h>

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
    int fibNum = faasm::getTypedInput<int>(2);
    int result = doFib(fibNum);

    std::string output = "Fibonacci " + std::to_string(fibNum) + " = " + std::to_string(result);
    const uint8_t *outputBuffer;
    outputBuffer = reinterpret_cast<const uint8_t *>(output.c_str());
    faasmSetOutput(outputBuffer, output.size());

    return 0;
}