#include "util.h"

#include <random>

namespace util {
    int randomInteger() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> uni(0, 10000000);

        int random_integer = uni(rng);

        return random_integer;
    }
}