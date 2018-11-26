#include "faasm/counter.h"

namespace faasm {

    void initCounter(FaasmMemory *memory, const char *counterKey) {
        auto counterBuffer = new uint8_t[1];
        counterBuffer[0] = 0;

        memory->writeState(counterKey, counterBuffer, 1);

        delete[] counterBuffer;
    }

    uint8_t getCounter(FaasmMemory *memory, const char *counterKey) {
        auto counterBuffer = new uint8_t[1];
        memory->readState(counterKey, counterBuffer, 1);

        uint8_t count = counterBuffer[0];

        delete[] counterBuffer;
        return count;
    }

    void incrementCounter(FaasmMemory *memory, const char *counterKey) {
        uint8_t count = getCounter(memory, counterKey);
        count++;

        auto counterBuffer = new uint8_t[1];
        counterBuffer[0] = count;
        memory->writeState(counterKey, counterBuffer, 1);

        delete[] counterBuffer;
    }

}

