#include "faasm/random.h"

#include <algorithm>

namespace faasm {
    int randomInteger(int iStart, int iEnd) {
        static std::random_device rd;
        static std::mt19937 rng(rd());

        std::uniform_int_distribution<int> uni(iStart, iEnd);
        int random_integer = uni(rng);

        return random_integer;
    }

    float randomFloat() {
        static std::random_device rd;
        static std::mt19937 rng(rd());

        // Distribution fixed here (can be static)
        static std::uniform_real_distribution<float> uni(0, 1);

        float f = uni(rng);
        return f;
    }

    void shuffleArray(int *arrayIn, size_t arrayLen) {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::shuffle(arrayIn, arrayIn + arrayLen, rng);
    }

    int* randomIntRange(int rangeLen) {
        int *range = new int[rangeLen];

        for(int i = 0; i < rangeLen; i++) {
            range[i] = i;
        }

        shuffleArray(range, rangeLen);

        return range;
    }
}