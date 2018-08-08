#include "emscripten.h"
#include <stdint.h>
#include <stdio.h>

EMSCRIPTEN_KEEPALIVE
int run(uint8_t *input, int inputLength, uint8_t *output, int outputLength) {
    printf("Args: %p %i %p %i", input, inputLength, output, outputLength);

    for (int i = 0; i < 5; i++) {
        uint8_t castValue = (uint8_t) i;
        printf("Writing %i at %i", castValue, i);
        output[i] = castValue;
    }

    return 0;
}