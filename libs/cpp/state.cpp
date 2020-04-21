#include <algorithm>
#include "faasm/core.h"
#include "faasm/state.h"


namespace faasm {
    void maskDouble(unsigned int *maskArray, unsigned long idx) {
        // NOTE - we assume int is half size of double
        unsigned long intIdx = 2 * idx;
        maskArray[intIdx] |= BIT_MASK_32;
        maskArray[intIdx + 1] |= BIT_MASK_32;
    }

    void zeroState(const char *key, size_t stateLen) {
        auto arr = new uint8_t[stateLen];
        std::fill(arr, arr + stateLen, 0);
        faasmWriteState(key, arr, stateLen);
        faasmPushState(key);
    }
}
