#include <faasm/faasm.h>
#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_shared_offset_example";

        uint8_t *chunkA = memory->readStateOffset(key, 7, 2, 3, true);
        uint8_t *chunkB = memory->readStateOffset(key, 7, 2, 3, true);

        // Write to memory only
        chunkA[0] = 6;
        chunkA[1] = 7;
        chunkA[2] = 8;

        // Check that shared state always points to same place
        bool pointersMatch = chunkA == chunkB;
        uint8_t output[1] = {(uint8_t) pointersMatch};
        memory->setOutput(output, 1);

        return 0;
    }
}