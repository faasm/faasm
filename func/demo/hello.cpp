#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *message = "Hello faasm!";

        memory->setOutput((uint8_t*) message, 12);

        return 0;
    }
};
