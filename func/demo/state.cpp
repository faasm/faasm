#include <faasm/faasm.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_example";

        // Check initial state size (zero when new state is created)
        long oldStateSize = memory->getStateSize(key);
        long newStateSize = oldStateSize + 1;

        // Create array with one extra element
        auto newState = new uint8_t[newStateSize];

        // Read existing state into this array
        memory->readState(key, newState, oldStateSize);

        // Add a new value to the end of the new state
        newState[newStateSize - 1] = (uint8_t) newStateSize;

        // Write the new state
        memory->writeState(key, newState, newStateSize);

        delete[] newState;

        return 0;
    }
}