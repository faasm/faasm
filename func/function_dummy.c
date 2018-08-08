#include "emscripten.h"
#include <stdint.h>

EMSCRIPTEN_KEEPALIVE
int run(uint8_t *input, int inputLength, uint8_t *output, int outputLength) {

    for (int i = 0; i < 5; i++) {
        output[i] = (uint8_t) i;
    }

    return 0;
}