#ifndef _FAASMC_H
#define _FAASMC_H

#include <string.h>
#include "faasm/core.h"

int main(int argc, char *argv[]) {
    // Invoke the entrypoint function
    exec(0);

    return 0;
}

#endif
