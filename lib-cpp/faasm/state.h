#ifndef FAASM_STATE_H
#define FAASM_STATE_H

#define BIT_MASK_8 0b11111111
#define BIT_MASK_32 0b11111111111111111111111111111111

namespace faasm {
    void maskDouble(unsigned int *maskArray, unsigned long idx);
}

#endif
