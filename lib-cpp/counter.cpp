#include "faasm/counter.h"
#include "faasm/core.h"

namespace faasm {
    void initCounter(const char *counterKey, bool async) {
        int counterBuffer[] = {0};
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);
        faasmWriteState(counterKey, counterBytes, sizeof(int), async);
    }

    int getCounter(const char *counterKey, bool async) {
        int counterBuffer[1];
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);
        faasmReadState(counterKey, counterBytes, sizeof(int), async);

        int count = counterBuffer[0];

        return count;
    }

    void incrementCounter(const char *counterKey, bool async) {
        int count = getCounter(counterKey, async);
        count++;

        int counterBuffer[1];
        counterBuffer[0] = count;
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);

        faasmWriteState(counterKey, counterBytes, sizeof(int), async);
    }

}

