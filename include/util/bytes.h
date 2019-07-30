#pragma once

#include <string>
#include <vector>

namespace util {
    std::vector<uint8_t> stringToBytes(const std::string &str);

    std::string bytesToString(const std::vector<uint8_t> &bytes);

    void trimTrailingZeros(std::vector<uint8_t> &vectorIn);

    int safeCopyToBuffer(const std::vector<uint8_t> &dataIn, uint8_t *buffer, int bufferLen);
}