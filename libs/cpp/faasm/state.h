#ifndef FAASM_STATE_H
#define FAASM_STATE_H

#include <stddef.h>

#define BIT_MASK_8 0b11111111
#define BIT_MASK_32 0b11111111111111111111111111111111

namespace faasm {
void maskDouble(unsigned int* maskArray, unsigned long idx);

void zeroState(const char* key, size_t stateLen);
}

#endif
