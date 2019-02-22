#include "random.h"

#include <random>

namespace util {
    int randomInteger(int iStart, int iEnd) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> uni(iStart, iEnd);

        int random_integer = uni(rng);

        return random_integer;
    }

    std::string randomString(int len) {
        char result[len];

        static const char alphanum[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < len; ++i) {
            result[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        result[len] = 0;

        return std::string(result);
    }
}