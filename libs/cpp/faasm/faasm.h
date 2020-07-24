#ifndef _FAASM_H
#define _FAASM_H

#include "faasm/core.h"

// For different integrations we need to provide different entry
// points. These must define their own main() method which ensures
// exec() eventually gets called
#if KNATIVE_NATIVE == 1
#include <knative_native/interface.h>
#else

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
