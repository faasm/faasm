#ifndef _FAASMC_H
#define _FAASMC_H

#include <string.h>
#include "faasm/core.h"

int main(int argc, char *argv[]) {
    // Invoke the entrypoint function
    int idx = faasmGetCurrentIdx();
    exec(idx);

    return 0;
}

#endif
