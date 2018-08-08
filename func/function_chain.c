#include "emscripten.h"
#include "faasm.h"

EMSCRIPTEN_KEEPALIVE
int run(uint8_t *input, int inputLength, uint8_t *output, int outputLength) {
    const char* name = "dummy";

    // Call into other function
    faasmCall(name, input, inputLength);

    return 0;
}