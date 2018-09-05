#include "faasm.h"

/**
 * Script to execute faasm functions natively
 */

int main() {
    uint8_t in[1024];
    uint8_t out[1024];
    uint8_t cFuncs[1024];
    uint8_t cIn[1024];

    // Call the actual function
    run(in, out, cFuncs, cIn);
}