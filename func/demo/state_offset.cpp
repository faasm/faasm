#include <faasm/faasm.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_offset_example";

        uint8_t value[7] = {0, 1, 2, 3, 4, 5, 6};
        memory->writeState(key, value, 7, false);

        // Now 5, 5, 5, 3, 4, 5, 6
        uint8_t partialA[3] = {5, 5, 5};
        memory->writeStateOffset(key, 7, 0, partialA, 3, false);

        // Now 5, 5, 6, 6, 4, 5, 6
        uint8_t partialB[2] = {6, 6};
        memory->writeStateOffset(key, 7, 2, partialB, 2, false);

        // Read 5, 5, 6, 6, 4
        uint8_t readValuePartial[5];
        memory->readStateOffset(key, 7, 0, readValuePartial, 5, false);

        memory->setOutput(readValuePartial, 5);

        return 0;
    }
}