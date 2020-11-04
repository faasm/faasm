#include <faasm/faasm.h>

FAASM_MAIN_FUNC()
{
    uint8_t inputBuf[3];
    faasmGetInput(inputBuf, 3);

    if (inputBuf[0] == 0) {
        // Expect 0, 1, 2
        if (inputBuf[1] != 1 || inputBuf[2] != 2) {
            return 1;
        }
    } else {
        // Expect 3, 4, 5
        if (inputBuf[0] != 3 || inputBuf[1] != 4 || inputBuf[2] != 5) {
            return 1;
        }
    }

    return 0;
}
