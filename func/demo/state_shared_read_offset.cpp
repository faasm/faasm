#include <faasm/faasm.h>

FAASM_MAIN_FUNC()
{
    const char* key = "state_shared_offset_example";
    uint8_t* actual = faasmReadStateOffsetPtr(key, 7, 2, 3);

    // Check that state has expected value
    uint8_t expected[7] = { 6, 7, 8 };
    bool valuesMatch = true;
    for (int i = 0; i < 3; i++) {
        if (actual[i] != expected[i]) {
            valuesMatch = false;
            break;
        }
    }

    uint8_t output[1] = { (uint8_t)valuesMatch };
    faasmSetOutput(output, 1);

    return 0;
}
