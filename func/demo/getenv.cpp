#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace faasm {
    char *printEnv(const char *varName) {
        char *val = getenv(varName);

        if (val == NULL) {
            printf("%s=NULL\n", varName);
        } else {
            printf("%s=%s\n", varName, val);
        }

        return val;
    }

    int exec(FaasmMemory *memory) {
        char *actual = printEnv("FULL_ASYNC");
        printEnv("LANG");
        printEnv("LC_CTYPE");
        printEnv("PYTHONHOME");

        uint8_t outputBuf[1];
        if (strcmp(actual, "1") == 0) {
            outputBuf[0] = 1;
        } else {
            outputBuf[0] = 0;
        }

        memory->setOutput(outputBuf, 1);
        return 0;
    }
}
