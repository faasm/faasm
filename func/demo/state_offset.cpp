#include <faasm/faasm.h>

FAASM_MAIN_FUNC() {
    const char *key = "state_offset_example";

    uint8_t value[7] = {0, 1, 2, 3, 4, 5, 6};
    faasmWriteState(key, value, 7);
    faasmPushState(key);

    // Now 5, 5, 5, 3, 4, 5, 6
    uint8_t partialA[3] = {5, 5, 5};
    faasmWriteStateOffset(key, 7, 0, partialA, 3);

    // Now 5, 5, 6, 6, 4, 5, 6
    uint8_t partialB[2] = {6, 6};
    faasmWriteStateOffset(key, 7, 2, partialB, 2);
    faasmPushState(key);

    // Read 5, 5, 6, 6, 4
    faasmPullState(key, 7);
    uint8_t readValuePartial[5];
    faasmReadStateOffset(key, 7, 0, readValuePartial, 5);

    faasmSetOutput(readValuePartial, 5);

    return 0;
}