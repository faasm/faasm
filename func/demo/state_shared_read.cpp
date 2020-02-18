#include <faasm/faasm.h>


FAASM_MAIN_FUNC() {
    const char *key = "state_shared_example";
    uint8_t *actual = faasmReadStatePtr(key, 7);

    // Check that state has expected value
    uint8_t expected[7] = {5, 5, 5, 5, 5, 5, 5};
    bool valuesMatch = true;
    for (int i = 0; i < 7; i++) {
        valuesMatch &= (actual[i] == expected[i]);
    }

    faasmSetOutput(BYTES(&valuesMatch), 1);
    return 0;
}