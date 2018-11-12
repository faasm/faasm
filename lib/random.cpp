#include "faasm/random.h"

#include <algorithm>

namespace faasm {
    int randomInteger(int iStart, int iEnd) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> uni(iStart, iEnd);

        int random_integer = uni(rng);

        return random_integer;
    }

    void shuffleArray(int *arrayIn, size_t arrayLen) {
        std::random_shuffle(arrayIn, arrayIn + arrayLen);
    }

    int* randomIntRange(size_t rangeLen) {
        int *range = new int[rangeLen];

        for(int i = 0; i < rangeLen; i++) {
            range[i] = i;
        }

        shuffleArray(range, rangeLen);

        return range;
    }
}