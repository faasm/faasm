#include "faasm/random.h"
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

    int incrementCounter(const char *counterKey, int increment, bool globalLock) {
        if(globalLock) {
            faasmLockStateWrite(counterKey);
        }

        int val = readIntState(counterKey);
        val += increment;
        writeIntState(counterKey, val);

        if(globalLock) {
            faasmUnlockStateWrite(counterKey);
        }

        return val;
    }

    AtomicInt::AtomicInt() {
        int rint = randomInteger(0, 10000);
        stateKey = "atomic_int_" + std::to_string(rint);
    }

    int AtomicInt::operator+=(int other) {
        value = incrementCounter(stateKey.c_str(), other, true);
        return value;
    }

    int AtomicInt::get() {
       return getCounter(stateKey.c_str());
    }
}

