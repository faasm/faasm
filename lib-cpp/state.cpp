#include "faasm/state.h"


namespace faasm {
    void maskDouble(unsigned int *maskArray, unsigned long idx) {
        // NOTE - we assume int is half size of double
        unsigned long intIdx = 2 * idx;
        maskArray[intIdx] |= BIT_MASK_32;
        maskArray[intIdx + 1] |= BIT_MASK_32;
    }
}
