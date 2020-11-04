#include "faasm/faasm.h"

#include <stdio.h>

FAASM_MAIN_FUNC()
{
    int stdInNo = fileno(stdin);
    int stdOutNo = fileno(stdout);
    int stdErrNo = fileno(stderr);

    printf("stdin=%i  stdout=%i  stderr=%i\n", stdInNo, stdOutNo, stdErrNo);

    return 0;
}
