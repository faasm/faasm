#ifndef _FAASM_H
#define _FAASM_H

// For lambda builds we need to include the lambda function interface
// which defines its own main method etc.
// TODO - work out how to do this in the new world
#if AWS_LAMBDA == 1
#include <lambda_func/interface.h>
#else

#include "faasm/core.h"

int main(int argc, char *argv[]) {
    // Invoke the main function
    exec(0);

    return 0;
}

#endif

#endif
