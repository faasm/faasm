#include "util.h"

namespace util {
    void intToBytes(const int* ints, int length, uint8_t* bytes) {
        for (int i = 0; i < length; i++) {
            for(int j = 0; j < 4; j++) {
                int byteIdx = (i * 4) + j;
                bytes[byteIdx] = uint8_t(ints[i] >> j * 8);
            }
        }
    }
}
