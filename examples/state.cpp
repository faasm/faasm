#include "faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char* key = "foo";

        uint8_t dummyState[5] = {0, 1, 2, 3, 4};

        memory->writeState(key, 0, dummyState, 5);

        uint8_t buffer[3];
        memory->readState(key, 2, buffer, 3);

        printf("Read: %02x %02x %02x\n", buffer[0], buffer[1], buffer[2]);

        return 0;
    }
}