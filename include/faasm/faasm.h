#include <stdint.h>
#include <emscripten.h>

#ifndef FAASM_FAASM_H
#define FAASM_FAASM_H

const int MAX_CHAINS = 100;

const int MAX_NAME_LENGTH = 20;
const int MAX_INPUT_BYTES = 1024 * 1024;

const int MAX_OUTPUT_BYTES = 1024 * 1024;

struct FaasmMemory {
    // Input data
    uint8_t *input;

    // Output data
    uint8_t *output;

    // Allows chaining of functions
    uint8_t **chainFunctions;
    uint8_t **chainInputs;
};

int exec(struct FaasmMemory memory);

int EMSCRIPTEN_KEEPALIVE run(uint8_t *input, uint8_t *output,
        uint8_t **chainFunctions, uint8_t **chainInputs
        ) {
    struct FaasmMemory memory;
    memory.input = input;
    memory.output = output;
    memory.chainFunctions = chainFunctions;
    memory.chainInputs = chainInputs;

    return exec(memory);
}

#endif