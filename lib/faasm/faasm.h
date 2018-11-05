#ifndef _FAASM_H
#define _FAASM_H

#include "faasm/memory.h"

namespace faasm {
    /**
    * Function for faasm functions to implement
    */
    int exec(FaasmMemory *memory);
}

// Entrypoint
int main(int argc, char* argv[], char **envp) {
    faasm::FaasmMemory memory;
    faasm::exec(&memory);

    return 0;
}

#endif
