#ifndef _FAASM_H
#define _FAASM_H

#include "faasm/core.h"

// For different integrations we need to provide different entry
// points. These must define their own main() method which ensures
// exec() eventually gets called
if KNATIVE_NATIVE == 1
#include <knative_native/interface.h>
#else

int main(int argc, char *argv[]) {
    int idx = faasmGetCurrentIdx();
    return exec(idx);
}

#endif

#endif
