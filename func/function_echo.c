#include "faasm.h"

/**
 * Writes the input to the output
 */
EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    for (int i = 0; i < MAX_INPUT_BYTES; i++) {
        memory->output[i] = memory->input[i];
    }

    return 0;
}