#include "faasm/input.h"
#include "faasm/core.h"

#include <stdint.h>
#include <string.h>
#include <string>

namespace faasm {
    int getIntInput(int defaultValue) {
        if(!faasmGetInputSize()) {
            return defaultValue;
        };

        uint8_t inputBuffer[sizeof(int)];
        faasmGetInput(inputBuffer, sizeof(int));
        char *strIn = reinterpret_cast<char *>(inputBuffer);

        return std::stoi(strIn);
    }

    void setStringOutput(const char* val) {
        auto bytesOutput = reinterpret_cast<const uint8_t *>(val);
        faasmSetOutput(bytesOutput, strlen(val));
    }

}