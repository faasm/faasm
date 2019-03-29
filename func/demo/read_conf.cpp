#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>


namespace faasm {
    int exec(FaasmMemory *memory) {
        char resultBuf[5];
        memory->readConfig("FULL_ASYNC", resultBuf);

        printf("FULL_ASYNC=%s\n", resultBuf);

        uint8_t outputBuf[1];
        if (strcmp(resultBuf, "1") == 0) {
            outputBuf[0] = 1;
        } else {
            outputBuf[0] = 0;
        }

        memory->setOutput(outputBuf, 1);
        return 0;
    }
}
