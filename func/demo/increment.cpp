#include "faasm/faasm.h"
#include "faasm/counter.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "incr_example";

        incrementCounter(memory, key);

        int count = getCounter(memory, key);

        // Return message
        char output[13];
        sprintf(output, "Counter: %03i", count);
        memory->setOutput((uint8_t *) output, 13);

        return 0;
    }
}