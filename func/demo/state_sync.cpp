#include <faasm/faasm.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_example";

        uint8_t value[4] = {0, 1, 2, 3};
        memory->writeState(key, value, 4, false);

        uint8_t readValue[4];
        memory->readState(key, readValue, 4, false);

        memory->setOutput(readValue, 4);

        return 0;
    }
}