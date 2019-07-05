#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>


FAASM_MAIN_FUNC() {
    char resultBuf[5];
    faasmReadConfig("FULL_ASYNC", resultBuf);

    printf("FULL_ASYNC=%s\n", resultBuf);

    uint8_t outputBuf[1];
    if (strcmp(resultBuf, "1") == 0) {
        outputBuf[0] = 1;
    } else {
        outputBuf[0] = 0;
    }

    faasmSetOutput(outputBuf, 1);
    return 0;
}
