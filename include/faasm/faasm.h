#include <stdint.h>
#include <string.h>
#include <emscripten.h>

#ifndef FAASM_FAASM_H
#define FAASM_FAASM_H

const int MAX_CHAINS = 50;
const int MAX_NAME_LENGTH = 32;
const int MAX_INPUT_BYTES = 1024;

/**
 * Main faasm memory abstraction
 */
struct FaasmMemory {
    // Input data
    uint8_t *input;

    // Output data
    uint8_t *output;

    // Allows chaining of functions
    uint8_t *chainFunctions;
    uint8_t *chainInputs;
    int32_t chainCount;
};

/**
 * Function for faasm functions to implement
 */
int exec(struct FaasmMemory *memory);

/**
 * Wrapper function used to abstract away the faasm memory management
 */
int EMSCRIPTEN_KEEPALIVE run(
        uint8_t *in,
        uint8_t *out,
        uint8_t *cFuncs,
        uint8_t *cIn
    ) {

    struct FaasmMemory memory;
    memory.input = in;
    memory.output = out;

    memory.chainFunctions = cFuncs;
    memory.chainInputs = cIn;
    memory.chainCount = 0;

    return exec(&memory);
}

/**
 * Adds a chain call to the given function
 */
void chainFunction(
        struct FaasmMemory *memory,
        char *name,
        uint8_t *inputData,
        int inputDataSize
    ) {
    // Work out how many chained functions we already have
    int32_t chainIdx = memory->chainCount;

    if(chainIdx > MAX_CHAINS) {
        fprintf(stderr, "%s", "Reached max chains");
        return;
    }

    // Get the memory offsets for name and data
    uint8_t *namePtr = memory->chainFunctions + (chainIdx * MAX_NAME_LENGTH);
    uint8_t *dataPtr = memory->chainInputs + (chainIdx * MAX_INPUT_BYTES);

    // Copy the data into place
    strcpy((char*) namePtr, name);
    memcpy(dataPtr, inputData, inputDataSize);

    // Increment the count
    memory->chainCount = chainIdx + 1;
}

#endif