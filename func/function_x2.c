#include "emscripten.h"
#include <stdint.h>

/**
 * Multiplies its array input by two and returns
 */
EMSCRIPTEN_KEEPALIVE
int run(uint8_t *input, int inputLength, uint8_t *output, int maxOutputLength) {
    for (int i = 0; i < inputLength; i++) {
        output[i] = input[i] * 2;
    }

    return 0;
}