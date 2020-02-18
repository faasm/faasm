#ifndef FAASM_INPUT_H
#define FAASM_INPUT_H

#include "faasm/core.h"

namespace faasm {
    template<typename T>
    T getTypedInput(T defaultValue) {
        if (faasmGetInputSize() == 0) {
            return defaultValue;
        }

        T inputBuff;
        faasmGetInput(BYTES(&inputBuff), sizeof(T));
        return inputBuff;
    }

    const char* getStringInput(const char* defaultValue);

    void setStringOutput(const char *val);

    int* parseStringToIntArray(const char* inStr, int expected);
}

#endif
