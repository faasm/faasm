#include <faasmc/faasm.h>

FAASM_MAIN_FUNC() {
    uint8_t output[5];
    for (int i = 0; i < 5; i++) {
        output[i] = (uint8_t) i;
    }

    faasmSetOutput(output, 5);

    return 0;
}