#pragma once

#include <string>
#include <vector>

namespace util {
    // Byte handling
    std::vector<uint8_t> stringToBytes(const std::string &str);

    void trimTrailingZeros(std::vector<uint8_t> &vectorIn);

    int safeCopyToBuffer(const std::vector<uint8_t> &dataIn, uint8_t *buffer, int bufferLen);
}