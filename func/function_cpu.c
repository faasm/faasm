#include "faasm.h"

/**
 * Does lots of work
 */
int exec(struct FaasmMemory *memory) {
    volatile unsigned long long i;
    for (i = 0; i < 5000000000ULL; ++i);

    return 0;
}