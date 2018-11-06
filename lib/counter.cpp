#include "faasm/counter.h"

namespace faasm {

    void initCounter(FaasmMemory *memory, const char* counterKey) {
        uint8_t counterBuffer[1] = {0};
        memory->writeState(counterKey, counterBuffer, 1);
    }

    uint8_t getCounter(FaasmMemory *memory, const char* counterKey) {
        uint8_t counterBuffer[1] = {0};
        memory->readState(counterKey, counterBuffer, 1);

        uint8_t count = counterBuffer[0];
        return count;
    }

    void incrementCounter(FaasmMemory *memory, const char* counterKey) {
        uint8_t count = getCounter(memory, counterKey);
        count++;

        uint8_t counterBuffer[1] = {count};
        memory->writeState(counterKey, counterBuffer, 1);
    }

}

