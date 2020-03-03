#include "faasm/faasm.h"

#include <stdio.h>
#include <faasm/input.h>


FAASM_MAIN_FUNC() {
    int inputVal = faasm::getTypedInput<int>(1);

    // Writing constants like this can result in a call to puts, not writev
    int i = 7;
    short s = 8;
    float f = 7.89;
    printf("i=%i s=%i f=%.2f\n", i, s, f);

    // This will always result in a call to writev
    double fA = 1.2345 * inputVal;
    double fB = 7.89101112 * inputVal;
    printf("FloatA=%.3f FloatB=%.3f\n", fA, fB);

    const char *strOut = "I am output";
    printf("Out: %s\n", strOut);
    printf("Unformatted output\n");

    faasm::setStringOutput("Normal Faasm output");

    return 0;
}
