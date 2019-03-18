#include "faasm/faasm.h"

#include <stdio.h>

namespace faasm {
    int increment() {
        static int i;
        i++;

        printf("Static int: %i\n", i);

        return i;
    }

    int exec(FaasmMemory *memory) {
        increment();
        increment();
        increment();
        int res =increment();

        // Take snapshot
        memory->snapshot("snapshot_restore");

        auto resBytes = reinterpret_cast<uint8_t *>(&res);
        memory->setOutput(resBytes, sizeof(int));

        return 0;
    }
}
