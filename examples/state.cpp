#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_example";

        // Check initial state size (zero when new state is created)
        size_t oldStateSize = memory->getStateSize(key);
        size_t newStateSize = oldStateSize + 1;

        // Create array with one extra element
        uint8_t newState[newStateSize];

        // Read existing state into this array
        memory->readState(key, newState, oldStateSize);

        // Add a new value to the end of the new state
        newState[newStateSize - 1] = (uint8_t) oldStateSize;

        // Write the new state
        memory->writeState(key, newState, newStateSize);

        return 0;
    }
}