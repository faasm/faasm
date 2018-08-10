#include "faasm.h"

EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    for (int i = 0; i < 5; i++) {
        uint8_t castValue = (uint8_t) i;
        memory->output[i] = castValue;
    }

    return 0;
}