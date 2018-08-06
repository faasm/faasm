#include "emscripten.h"
#include <stdio.h>

/**
 * Multiplies its array input by two and returns
 */
int* EMSCRIPTEN_KEEPALIVE run(int *inputData, int length) {
    int result[length];

    for (int i = 0; i < length; i++) {
        result[i] = inputData[i] * 2;
    }

    return result;
}
