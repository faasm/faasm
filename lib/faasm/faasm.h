#ifndef _FAASM_H
#define _FAASM_H

#include "faasm/memory.h"

// Work out if we're in a wasm cross-compile environment
// This can be used to avoid functions being removed by DCE
#if __clang_major__ == 8

#define FAASM_EXPORT __attribute__((used)) __attribute__ ((visibility ("default")))
#define FAASM_INLINE
#else

#define FAASM_EXPORT
#define FAASM_INLINE static inline
#endif

namespace faasm {
    /**
    * Function for faasm functions to implement
    */
    int exec(FaasmMemory *memory);
}

// Things to keep the compiler happy
FAASM_INLINE
FAASM_EXPORT
int run() {
    faasm::FaasmMemory memory;

    return faasm::exec(&memory);
}

#if __clang_major__ == 8
// Dummy main function to keep compiler happy
int main(int a, char* args[]) {
    run();

    return 0;
}
#endif

#endif
