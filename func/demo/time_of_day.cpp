#include "faasm/faasm.h"

#include <sys/time.h>
#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        timeval tv{};
        gettimeofday(&tv, nullptr);

        size_t strLen = 8 + sizeof(long int);
        char str[strLen];
        sprintf(str, "Seconds: %li", tv.tv_sec);

        auto output = reinterpret_cast<uint8_t *>(str);
        memory->setOutput(output, strLen);

        return 0;
    }
}