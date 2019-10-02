#include "bytes.h"

#include <vector>

namespace util {

    std::vector<uint8_t> stringToBytes(const std::string &str) {
        if(str.empty()) {
            std::vector<uint8_t> empty;
            return empty;
        }

        // Get raw data as byte pointer
        const char *cstr = str.c_str();
        auto *rawBytes = reinterpret_cast<const uint8_t *>(cstr);

        // Wrap in bytes vector
        std::vector<uint8_t> actual(rawBytes, rawBytes + str.length());
        return actual;
    }

    void trimTrailingZeros(std::vector<uint8_t> &vectorIn) {
        long i = vectorIn.size() - 1;

        while (i >= 0 && vectorIn.at((unsigned long) i) == 0) {
            i--;
        }

        if (i < 0) {
            vectorIn.clear();
        } else {
            vectorIn.resize((unsigned long) i + 1);
        }
    }

    int safeCopyToBuffer(const std::vector<uint8_t> &dataIn, uint8_t *buffer, int bufferLen) {
        int dataSize = (int) dataIn.size();

        if (bufferLen <= 0) {
            return dataSize;
        }

        if(dataIn.empty()) {
            return 0;
        }

        // Handle short buffer
        if (dataSize > bufferLen) {
            dataSize = bufferLen;
        }

        std::copy(dataIn.data(), dataIn.data() + dataSize, buffer);

        return dataSize;
    }

    std::string bytesToString(const std::vector<uint8_t> &bytes) {
        unsigned long byteLen = bytes.size();
        const char *charPtr = reinterpret_cast<const char *>(bytes.data());
        const std::string result = std::string(charPtr, charPtr + byteLen);

        return result;
    }
}
