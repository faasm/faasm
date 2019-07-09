#include <faasm/faasm.h>
#include <stdio.h>

FAASM_MAIN_FUNC() {
    const char *key = "state_shared_example";
    uint8_t *actual = faasmReadStatePtr(key, 7, true);

    // Check that state has expected value
    uint8_t expected[7] = {5, 5, 5, 5, 5, 5, 5};
    bool valuesMatch = true;
    for (int i = 0; i < 7; i++) {
        if (actual[i] != expected[i]) {
            valuesMatch = false;
            break;
        }
    }

    uint8_t output[1] = {(uint8_t) valuesMatch};
    faasmSetOutput(output, 1);

    return 0;
}