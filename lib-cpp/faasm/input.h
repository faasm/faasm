#ifndef FAASM_INPUT_H
#define FAASM_INPUT_H

namespace faasm {
    int getIntInput(int defaultValue);

    const char* getStringInput(const char* defaultValue);

    void setStringOutput(const char *val);

    int* parseStringToIntArray(const char* inStr, int expected);
}

#endif
