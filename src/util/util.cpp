#include "util.h"
#include <string>
#include <random>

namespace util {
    std::string getEnvVar(std::string const &key, std::string const &dflt) {
        char const *val = getenv(key.c_str());
        return val == nullptr ? dflt : std::string(val);
    }

    int randomInteger() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> uni(0, 10000000); // guaranteed unbiased

        int random_integer = uni(rng);

        return random_integer;
    }
}