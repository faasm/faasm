#ifndef _FAASM_H
#define _FAASM_H

// For lambda builds we need to include the lambda function interface
// which defines its own main method etc.
#if AWS_LAMBDA == 1
#include <lambda_func/interface.h>
#else

#if WASM_BUILD == 1
// Hack to override getenv to force using our virtualised environment
#define getenv _getenv
#endif

#include "faasm/memory.h"

namespace faasm {
    /**
    * Function for faasm functions to implement
    */
    int exec(FaasmMemory *memory);
}

int main(int argc, char *argv[]) {
    faasm::FaasmMemory memory;
    faasm::exec(&memory);

    return 0;
}

#endif

#endif
