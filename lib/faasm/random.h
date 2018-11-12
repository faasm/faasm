#ifndef FAASM_RANDOM_H
#define FAASM_RANDOM_H

#include <random>

namespace faasm {
    int randomInteger(int iStart, int iEnd);

    void shuffleArray(int *arrayIn, size_t arrayLen);

    int* randomIntRange(size_t rangeLen);
}

#endif
