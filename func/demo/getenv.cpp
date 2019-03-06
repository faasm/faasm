#include "faasm/faasm.h"

#include <stdio.h>
#include <stdlib.h>

namespace faasm {
    void printEnv(const char *varName) {
        char *val = getenv(varName);

        if (val == NULL) {
            printf("%s=NULL\n", varName);
        } else {
            printf("%s=%s\n", varName, val);
        }
    }

    int exec(FaasmMemory *memory) {
        printEnv("FULL_ASYNC");
        printEnv("LANG");
        printEnv("LC_CTYPE");
        printEnv("PYTHONHOME");

        return 0;
    }
}
