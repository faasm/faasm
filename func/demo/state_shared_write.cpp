#include <faasm/faasm.h>
#include <stdio.h>

FAASM_MAIN_FUNC() {
    const char *key = "state_shared_example";

    uint8_t *actualA = faasmReadStatePtr(key, 7, true);
    uint8_t *actualB = faasmReadStatePtr(key, 7, true);

    // Write to memory only
    for (int i = 0; i < 7; i++) {
        actualA[i] = 5;
    }

    // Check that shared state always points to same place
    bool pointersMatch = actualA == actualB;
    uint8_t output[1] = {(uint8_t) pointersMatch};
    faasmSetOutput(output, 1);

    return 0;
}