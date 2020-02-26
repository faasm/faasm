#include "faasm/faasm.h"

#include <stdio.h>
#include <faasm/input.h>

FAASM_MAIN_FUNC() {
    int i = 7;
    short s = 8;
    float f = 7.89;
    printf("i=%i s=%i f=%.2f\n", i, s, f);

    const char* strOut = "I am output";
    printf("Out: %s\n", strOut);
    fflush(stdout);

    printf("Unformatted output\n");
    fflush(stdout);

    faasm::setStringOutput("Normal Faasm output");

    return 0;
}
