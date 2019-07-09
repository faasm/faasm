#include <faasm/faasm.h>

FAASM_MAIN_FUNC() {
    const char *key = "state_example";

    uint8_t value[4] = {0, 1, 2, 3};
    faasmWriteState(key, value, 4, false);

    uint8_t readValue[4];
    faasmReadState(key, readValue, 4, false);

    faasmSetOutput(readValue, 4);

    return 0;
}