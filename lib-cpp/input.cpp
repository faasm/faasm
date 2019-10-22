#include "faasm/input.h"
#include "faasm/core.h"

#include <stdint.h>
#include <string.h>
#include <string>

namespace faasm {
    const char *getStringInput(const char *defaultValue) {
        long inputSize = faasmGetInputSize();
        if (inputSize == 0) {
            return defaultValue;
        }

        auto inputBuffer = new uint8_t[inputSize + 1];
        faasmGetInput(inputBuffer, inputSize);

        // Force null-termination
        inputBuffer[inputSize] = 0;

        char *strIn = reinterpret_cast<char *>(inputBuffer);

        return strIn;
    }

    void setStringOutput(const char *val) {
        auto bytesOutput = reinterpret_cast<const uint8_t *>(val);
        faasmSetOutput(bytesOutput, strlen(val));
    }

    int *parseStringToIntArray(const char *strIn, int nInts) {
        char *strCopy = new char[strlen(strIn)];
        strcpy(strCopy, strIn);

        char *nextSubstr = strtok(strCopy, " ");
        int *result = new int[nInts];

        int i = 0;
        while (nextSubstr != NULL) {
            result[i] = std::stoi(nextSubstr);
            nextSubstr = strtok(NULL, " ");
            i++;
        }

        return result;
    }
}