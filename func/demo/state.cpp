#include <faasm/faasm.h>
#include "stdio.h"


namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_example";

        // Check initial state size (zero when new state is created)
        uint8_t *ptr = memory->readStatePtr(key, 6, true);

        for(int i = 0; i < 6; i++) {
            printf("%i: %i\n", i, ptr[i]);
        }

        printf("%i\n", *(ptr + 4147483647));

        return 0;
    }
}