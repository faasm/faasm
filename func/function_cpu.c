#include "faasm.h"

/**
 * Does lots of work
 */
EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    volatile unsigned long long i;
    for (i = 0; i < 5000000000ULL; ++i);

    return 0;
}