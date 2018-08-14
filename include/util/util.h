#pragma once

#include <string>
#include <vector>

namespace util {
    std::string getEnvVar(std::string const &key, std::string const &dflt);

    int randomInteger();

    std::vector<uint8_t> stringToBytes(const std::string &str);

    void trimTrailingZeros(std::vector<uint8_t> &vectorIn);
}
