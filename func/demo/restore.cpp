#include "faasm/faasm.h"

#include <stdio.h>

int increment() {
    static int i;
    i++;

    printf("Static int: %i\n", i);

    return i;
}

FAASM_MAIN_FUNC() {
    // Restore from snapshot
    faasmRestore("snapshot_restore");

    increment();
    increment();
    int res = increment();

    auto resBytes = reinterpret_cast<uint8_t *>(&res);
    faasmSetOutput(resBytes, sizeof(int));

    return 0;
}
