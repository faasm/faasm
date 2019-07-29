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
        int dataInSize = (int) dataIn.size();

        if (bufferLen <= 0) {
            return dataInSize;
        }

        if(dataIn.empty()) {
            return 0;
        }

        // Handle buffer longer than actual data
        int dataLen = bufferLen;
        if (dataInSize < bufferLen) {
            dataLen = dataInSize;
        }

        std::copy(dataIn.data(), dataIn.data() + dataLen, buffer);

        return dataInSize;
    }
}
