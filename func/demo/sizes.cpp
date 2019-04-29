#include "faasm/faasm.h"

#include <iostream>
#include <typeinfo>

namespace faasm {
    template<typename T>
    void checkSizeOf(size_t expected, const char * name) {
        size_t actual = sizeof(T);
        if(actual != expected) {
            printf("Size of %s not as expected (expected %lu was %lu)\n", name, expected, actual);
            exit(1);
        }
    }
            
    int exec(FaasmMemory *memory) {
        checkSizeOf<double>(8, "double");
        checkSizeOf<float>(4, "float");
        checkSizeOf<int>(4, "int");
        checkSizeOf<long>(4, "long");
        checkSizeOf<long long>(8, "long long");
        checkSizeOf<short>(2, "short");
        checkSizeOf<char>(1, "char");

        checkSizeOf<off_t>(8, "off_t");
        checkSizeOf<uintptr_t>(4, "uintptr_t");
        checkSizeOf<intptr_t>(4, "intptr_t");
        checkSizeOf<size_t>(4, "size_t");
        checkSizeOf<void *>(4, "void *");

        return 0;
    }
}
