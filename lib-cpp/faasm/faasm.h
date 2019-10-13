#ifndef _FAASM_H
#define _FAASM_H

// For different integrations we need to provide different entry
// points. These must define their own main() method which ensures
// exec() eventually gets called
#if AWS_LAMBDA == 1

#include <lambda_func/interface.h>
#elif KNATVE_NATIVE == 1

#include <knative_native/interface.h>
#else

#include "faasm/core.h"

int main(int argc, char *argv[]) {
    int idx = faasmGetCurrentIdx();
    return exec(idx);
}

#endif

#endif
