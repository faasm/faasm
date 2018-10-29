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

    const char* url = "www.google.com";
    std::vector<uint8_t> bytes = util::stringToBytes(url);

    // Call the actual function
    run(bytes.data(), outputData, chainFuncs, chainInputData);

    // Print the output data
    printf("\nOutput: ");
    printBytes(outputData, MAX_INPUT_BYTES);
    printf("Chains: ");
    printBytes(chainFuncs, MAX_CHAINS * MAX_NAME_LENGTH);
    printf("Chain data: ");
    printBytes(chainInputData, MAX_CHAINS * MAX_INPUT_BYTES);
}