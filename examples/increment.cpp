#include "faasm.h"
#include "counter.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "incr_example";

        incrementCounter(memory, key);

        uint8_t count = getCounter(memory, key);

        // Return message
        char output[12];
        sprintf(output, "Counter: %i", count);
        memory->setOutput((uint8_t *) output, 12);

        return 0;
    }
}