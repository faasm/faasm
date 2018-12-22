#include <faasm/faasm.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_async_example";

        // Read and write async
        uint8_t value[4] = {3, 2, 1, 0};
        memory->writeState(key, value, 4, true);

        uint8_t readValueAsync[4];
        memory->readState(key, readValueAsync, 4, true);

        // Push synchronously and read again
        memory->pushState(key);
        uint8_t readValueSync[4];
        memory->readState(key, readValueSync, 4);

        // Check things are equal
        uint8_t equal[4];
        for(int i = 0; i < 4; i++) {
            bool isEqual = readValueAsync[i] == readValueSync[i];
            equal[i] = uint8_t (isEqual);
        }

        memory->setOutput(equal, 4);

        return 0;
    }
}