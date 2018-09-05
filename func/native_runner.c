#include "faasm.h"

/**
 * Script to execute faasm functions natively
 */

int main() {
    uint8_t inputData[MAX_INPUT_BYTES];
    uint8_t outputData[MAX_INPUT_BYTES];
    uint8_t chainFuncs[MAX_CHAINS];
    uint8_t chainInputData[MAX_CHAINS * MAX_INPUT_BYTES];

    // Call the actual function
    run(inputData, outputData, chainFuncs, chainInputData);
}