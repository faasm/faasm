#include "faasm/faasm.h"

#include <stdio.h>
#include <faasm/input.h>
#include <faasm/random.h>

FAASM_MAIN_FUNC() {
    // Writing constants like this can result in a call to puts, not writev
    int i = 7;
    short s = 8;
    float f = 7.89;
    printf("i=%i s=%i f=%.2f stdout\n", i, s, f);

    // This will always result in a call to writev
    float fA = faasm::randomFloat();
    float fB = faasm::randomFloat();
    printf("FloatA=%.3f FloatB=%.3f stdout\n", fA, fB);

    fflush(stdout);

    const char* strOut = "I am output";
    printf("Out: %s\n", strOut);
    printf("Unformatted stdout\n");

    faasm::setStringOutput("Normal Faasm stdout");

    return 0;
}
