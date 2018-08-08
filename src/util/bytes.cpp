#include "util.h"

#include <vector>

namespace util {

    std::vector<uint8_t> stringToBytes(const std::string &str) {
        // Get raw data as byte pointer
        const char *cstr = str.c_str();
        auto *rawBytes = reinterpret_cast<const uint8_t*>(cstr);

        // Put into a vector
        std::vector<uint8_t> actual(rawBytes, rawBytes + str.length());

        return actual;
    }
}
