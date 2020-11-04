#include "faasm/faasm.h"

#include <iostream>
#include <typeinfo>

template<typename T>
void checkSizeOf(size_t expected, const char* name)
{
    size_t actual = sizeof(T);
    if (actual != expected) {
        printf("Size of %s not as expected (expected %lu was %lu)\n",
               name,
               expected,
               actual);
        exit(1);
    }
}

FAASM_MAIN_FUNC()
{
    checkSizeOf<double>(8, "double");

    checkSizeOf<float>(4, "float");
    checkSizeOf<int>(4, "int");

#ifdef __wasm__
    checkSizeOf<long>(4, "long");
    checkSizeOf<long long>(8, "long long");
#else
    checkSizeOf<long>(8, "long");
    checkSizeOf<long long>(8, "long long");
#endif

    checkSizeOf<short>(2, "short");
    checkSizeOf<char>(1, "char");
    checkSizeOf<off_t>(8, "off_t");

#ifdef __wasm__
    int ptrSize = 4;
#else
    int ptrSize = 8;
#endif

    checkSizeOf<uintptr_t>(ptrSize, "uintptr_t");
    checkSizeOf<intptr_t>(ptrSize, "intptr_t");
    checkSizeOf<void*>(ptrSize, "void *");
    checkSizeOf<size_t>(ptrSize, "size_t");

    return 0;
}
