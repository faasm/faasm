#include "faasm/core.h"
#include "faasm/state.h"

#include <stdint.h>

namespace faasm {
    void writeIntState(const char* key, int val) {
        auto ptr = reinterpret_cast<uint8_t*>(&val);
        faasmWriteState(key, ptr, sizeof(int), false);
    }

    int readIntState(const char *key) {
        int val;
        auto buf = reinterpret_cast<uint8_t *>(&val);
        faasmReadState(key, buf, sizeof(int), false);
        return val;
    }


}