#include "faasm/counter.h"
#include "faasm/core.h"

namespace faasm {
    void writeIntState(const char* key, int val) {
        auto ptr = reinterpret_cast<uint8_t*>(&val);
        faasmWriteState(key, ptr, sizeof(int));
    }

    int readIntState(const char *key) {
        int val;
        auto buf = reinterpret_cast<uint8_t *>(&val);
        faasmReadState(key, buf, sizeof(int));
        return val;
    }

    void initCounter(const char *counterKey) {
        int counterBuffer[] = {0};
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);
        faasmWriteState(counterKey, counterBytes, sizeof(int));
    }

    int getCounter(const char *counterKey) {
        int counterBuffer[1];
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);
        faasmReadState(counterKey, counterBytes, sizeof(int));

        int count = counterBuffer[0];

        return count;
    }

    void incrementCounter(const char *counterKey) {
        int count = getCounter(counterKey);
        count++;

        int counterBuffer[1];
        counterBuffer[0] = count;
        auto counterBytes = reinterpret_cast<uint8_t *>(counterBuffer);

        faasmWriteState(counterKey, counterBytes, sizeof(int));
    }

    void incrementCounter(const char *counterKey, int increment, bool globalLock) {
        if(globalLock) {
            faasmLockStateWrite(counterKey);
        }

        int val = readIntState(counterKey);
        val += increment;
        writeIntState(counterKey, val);

        if(globalLock) {
            faasmUnlockStateWrite(counterKey);
        }
    }
}

