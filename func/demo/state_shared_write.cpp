#include <faasm/faasm.h>
#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_shared_example";

        uint8_t *actualA = memory->readState(key, 7, true);
        uint8_t *actualB = memory->readState(key, 7, true);

        // Write to memory only
        for (int i = 0; i < 7; i++) {
            actualA[i] = 5;
        }

        // Check that shared state always points to same place
        bool pointersMatch = actualA == actualB;
        uint8_t output[1] = {(uint8_t) pointersMatch};
        memory->setOutput(output, 1);

        return 0;
    }
}