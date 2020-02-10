#ifndef FAASM_PRINT_H
#define FAASM_PRINT_H

#include <stdio.h>

namespace faasm {
    char *floatToStr(float f);

    void printArray(int *data, size_t size);
}

#endif
