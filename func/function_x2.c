#include "faasm.h"

/**
 * Multiplies its array input by two
 */
int exec(struct FaasmMemory *memory) {
    for (int i = 0; i < MAX_INPUT_BYTES; i++) {
        memory->output[i] = memory->input[i] * 2;
    }

    return 0;
}