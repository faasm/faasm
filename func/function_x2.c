#include "emscripten.h"

/**
 * Multiplies its array input by two and returns
 */
EMSCRIPTEN_KEEPALIVE
int run(int *input, int inputLength, int *output, int maxOutputLength) {
    for (int i = 0; i < inputLength; i++) {
        output[i] = input[i] * 2;
    }

    return 0;
}