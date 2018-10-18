#include "faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "incr_example";

        // Check initial state size (zero when new state is created)
        size_t stateSize = memory->getStateSize(key);

        // Either initialise or read state
        uint8_t newState[1];
        if (stateSize == 0) {
            newState[0] = 0;
        } else {
            memory->readState(key, newState, 1);
        }

        // Increment counter
        newState[0]++;

        // Write state
        memory->writeState(key, newState, 1);

        // Return message
        char output[12];
        sprintf(output, "Counter: %i", newState[0]);
        memory->setOutput((uint8_t *) output, 12);

        return 0;
    }
}