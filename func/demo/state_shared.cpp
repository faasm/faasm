#include <faasm/faasm.h>
#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_shared_example";

        uint8_t *actual = memory->readState(key, 7, true);

        for (int i = 0; i < 7; i++) {
            printf("%i : %i\n", i, actual[i]);
        }

        return 0;
    }
}