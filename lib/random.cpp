#include "faasm/random.h"

namespace faasm {
    int randomInteger(int iStart, int iEnd) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> uni(iStart, iEnd);

        int random_integer = uni(rng);

        return random_integer;
    }
}
