#include "emscripten.h"
#include "faasm.h"

EMSCRIPTEN_KEEPALIVE
int run(uint8_t *input, int inputLength, uint8_t *output, int outputLength) {
    const char* user = "jimmy";
    const char* func = "dummy";

    // Call into other function
    faasmCall(user, func, input, inputLength);

    return 0;
}