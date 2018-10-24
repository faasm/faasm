#include "faasm.h"
#include <util/util.h>
#include <algorithm>

/**
 * Script to execute faasm functions natively
 */

void printBytes(uint8_t *ptr, int count) {
    printf("[");

    int gapCount = 0;
    for (int i = 0; i < count; i++) {
        if (ptr[i] == 0) {
            gapCount++;
            continue;
        }

        if (gapCount != 0) {
            printf(" 00x%i ", gapCount);
            gapCount = 0;
        }

        printf("%02x", ptr[i]);
    }

    printf("]\n");
}

int main() {
    uint8_t outputData[MAX_INPUT_BYTES];
    uint8_t chainFuncs[MAX_CHAINS * MAX_NAME_LENGTH];
    uint8_t chainInputData[MAX_CHAINS * MAX_INPUT_BYTES];

    int i = util::randomInteger(0, 900);
    int inputData[3] = {i, i+100, 10};
    auto inputBytes = reinterpret_cast<uint8_t *>(inputData);

    // Call the actual function
    run(inputBytes, outputData, chainFuncs, chainInputData);

    // Print the output data
    printf("\nOutput: ");
    printBytes(outputData, MAX_INPUT_BYTES);
    printf("Chains: ");
    printBytes(chainFuncs, MAX_CHAINS * MAX_NAME_LENGTH);
    printf("Chain data: ");
    printBytes(chainInputData, MAX_CHAINS * MAX_INPUT_BYTES);
}