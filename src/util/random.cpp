#include "random.h"

#include <random>

namespace util {
    std::string randomString(int len) {
        char result[len];

        static const char alphanum[] =
                "123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

        static std::random_device rd;
        static std::mt19937 rng(rd());

        // Range cannot include last element of alphanum array as this is a null terminator
        std::uniform_int_distribution<int> uni(0, sizeof(alphanum) - 2);

        for (int i = 0; i < len; ++i) {
            int r = uni(rng);
            result[i] = alphanum[r];
        }

        return std::string(result, result + len);
    }

    std::string randomStringFromSet(const std::unordered_set<std::string> &s) {
        static std::random_device rd;
        static std::mt19937 rng(rd());

        unsigned long nElems = s.size();
        if (nElems == 0) {
            return "";
        }

        std::uniform_int_distribution<int> uni(0, nElems - 1);
        int random_integer = uni(rng);
        auto it = s.begin();
        std::advance(it, random_integer);

        return *it;
    }
}