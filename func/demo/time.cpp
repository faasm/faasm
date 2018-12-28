#include "faasm/faasm.h"
#include "faasm/time.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        double secs = getSecondsSinceEpoch();

        size_t strLen = 8 + sizeof(double);
        char str[strLen];
        sprintf(str, "Seconds: %.3f", secs);

        auto output = reinterpret_cast<uint8_t *>(str);
        memory->setOutput(output, strLen);

        return 0;
    }
}