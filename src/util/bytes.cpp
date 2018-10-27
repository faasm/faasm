#include "util.h"

#include <vector>

namespace util {

    std::vector<uint8_t> stringToBytes(const std::string &str) {
        // Get raw data as byte pointer
        const char *cstr = str.c_str();
        auto *rawBytes = reinterpret_cast<const uint8_t *>(cstr);

        // Put into a vector
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

    void printBytes(uint8_t *ptr, size_t count, bool asChar) {
        printf("[");

        for (int i = 0; i < count; i++) {
            if(asChar) {
                printf("%c ", ptr[i]);
            }
            else {
                printf("%02x ", ptr[i]);
            }
        }

        printf("]\n");
    }

    int safeCopyToBuffer(std::vector<uint8_t> &state, uint8_t *buffer, int bufferLen) {
        int stateSize = (int) state.size();

        if (bufferLen <= 0) {
            return stateSize;
        }

        // Handle buffer longer than state size
        int dataLen = bufferLen;
        if (stateSize < bufferLen) {
            dataLen = stateSize;
        }

        std::copy(state.data(), state.data() + dataLen, buffer);

        return stateSize;
    }
}
