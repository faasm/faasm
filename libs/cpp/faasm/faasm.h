#ifndef _FAASM_H
#define _FAASM_H

#include "faasm/core.h"

// Hooks for handling normal argc/argv
int faasm_argc;
char** faasm_argv;

int main(int argc, char *argv[]) {
    faasm_argc = argc;
    faasm_argv = argv;

    int idx = faasmGetCurrentIdx();
    return exec(idx);
}

#endif

#endif
