#include "faasm/counter.h"

namespace faasm {

    void initCounter(FaasmMemory *memory, const char *counterKey, bool async) {
        int counterBuffer[] = {0};
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);
        memory->writeState(counterKey, counterBytes, sizeof(int), async);
    }

    int getCounter(FaasmMemory *memory, const char *counterKey, bool async) {
        int counterBuffer[1];
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);
        memory->readState(counterKey, counterBytes, sizeof(int), async);

        int count = counterBuffer[0];

        return count;
    }

    void incrementCounter(FaasmMemory *memory, const char *counterKey, bool async) {
        int count = getCounter(memory, counterKey);
        count++;

        int counterBuffer[1];
        counterBuffer[0] = count;
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);

        memory->writeState(counterKey, counterBytes, sizeof(int), async);
    }

}

