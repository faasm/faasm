#ifndef FAASM_PRINT_H
#define FAASM_PRINT_H

#include <stdio.h>

namespace faasm {
    char *floatToStr(float f);

    template <typename T>
    void printArray(T *data, size_t size);
}

#endif
