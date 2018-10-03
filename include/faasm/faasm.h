#ifndef _FAASM_H
#define _FAASM_H 1

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHAINS 50
#define MAX_NAME_LENGTH 32
#define MAX_INPUT_BYTES 65536

// Work out if we're in a wasm cross-compile environment
// This can be used to avoid functions being removed by DCE
#if __clang_major__ == 8
#define FAASM_INLINE __attribute__((used)) __attribute__ ((visibility ("default")))
#else
#define FAASM_INLINE static inline
#endif

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
FAASM_INLINE
int run(
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
FAASM_INLINE
void chainFunction(
        struct FaasmMemory *memory,
        char *name,
        uint8_t *inputData,
        int inputDataSize
) {
    // Work out how many chained functions we already have
    int32_t chainIdx = memory->chainCount;

    if (chainIdx > MAX_CHAINS) {
        fprintf(stderr, "%s", "Reached max chains");
        return;
    }

    // Get the memory offsets for name and data
    uint8_t *namePtr = memory->chainFunctions + (chainIdx * MAX_NAME_LENGTH);
    uint8_t *dataPtr = memory->chainInputs + (chainIdx * MAX_INPUT_BYTES);

    // Copy the data into place
    strcpy((char *) namePtr, name);
    memcpy(dataPtr, inputData, (size_t) inputDataSize);

    // Increment the count
    memory->chainCount = chainIdx + 1;
}

#if __clang_major__ == 8
// Cross-compiler needs an entry point function (which will never be called)
int main(int a, char* args[]) {
    return 0;
}
#endif

#endif