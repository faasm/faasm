#include "faasm/faasm.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    int stdInNo = fileno(stdin);
    int stdOutNo = fileno(stdout);
    int stdErrNo = fileno(stderr);

    printf("stdin=%i  stdout=%i  stderr=%i\n", stdInNo, stdOutNo, stdErrNo);

    return 0;
}
