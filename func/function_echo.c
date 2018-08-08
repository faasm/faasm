#include "emscripten.h"
#include <stdint.h>

/**
 * Writes the input to the output
 */
EMSCRIPTEN_KEEPALIVE
int run(uint8_t *input, int inputLength, uint8_t *output, int maxOutputLength) {
    for (int i = 0; i < inputLength; i++) {
        output[i] = input[i];
    }

    return 0;
}