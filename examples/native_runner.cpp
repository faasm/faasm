#include "faasm.h"

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

static uint8_t _state[1024 * 1024];

size_t __faasm_read_state(const char *key, uint8_t *buffer, size_t bufferLen) {
    std::copy(_state, _state + bufferLen, buffer);

    return bufferLen;
}

void __faasm_write_state(const char *key, uint8_t *data, size_t dataLen) {
    std::copy(data, data + dataLen, _state);
}

void __faasm_write_state_offset(const char *key, size_t offset, uint8_t *data, size_t dataLen) {
    std::copy(data, data + dataLen, _state + offset);
}

void _faasm_read_state_offset(const char *key, size_t offset, uint8_t *buffer, size_t bufferLen) {
    std::copy(_state + offset, _state + offset + bufferLen, buffer);
}


int main() {
    uint8_t inputData[MAX_INPUT_BYTES];
    uint8_t outputData[MAX_INPUT_BYTES];
    uint8_t chainFuncs[MAX_CHAINS * MAX_NAME_LENGTH];
    uint8_t chainInputData[MAX_CHAINS * MAX_INPUT_BYTES];

    // Set up some input data
    strcpy((char *) inputData, "www.google.com");

    // Call the actual function
    run(inputData, outputData, chainFuncs, chainInputData);

    // Print the output data
    printf("\nOutput: ");
    printBytes(outputData, MAX_INPUT_BYTES);
    printf("Chains: ");
    printBytes(chainFuncs, MAX_CHAINS * MAX_NAME_LENGTH);
    printf("Chain data: ");
    printBytes(chainInputData, MAX_CHAINS * MAX_INPUT_BYTES);
}