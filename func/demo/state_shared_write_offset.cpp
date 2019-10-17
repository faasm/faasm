#include <faasm/faasm.h>
#include <stdio.h>

FAASM_MAIN_FUNC() {
    const char *key = "state_shared_offset_example";

    uint8_t *chunkA = faasmReadStateOffsetPtr(key, 7, 2, 3);
    uint8_t *chunkB = faasmReadStateOffsetPtr(key, 7, 2, 3);

    // Write to memory only
    chunkA[0] = 6;
    chunkA[1] = 7;
    chunkA[2] = 8;

    // Check that shared state always points to same place
    bool pointersMatch = chunkA == chunkB;
    uint8_t output[1] = {(uint8_t) pointersMatch};
    faasmSetOutput(output, 1);

    return 0;
}