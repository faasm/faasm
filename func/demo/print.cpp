#include "faasm/faasm.h"
#include "faasm/print.h"

#include <stdio.h>

FAASM_MAIN_FUNC() {
    short s = 8;
    printf("s: %i\n", s);

    int i = 7;
    printf("i: %i\n", i);

    float f = 7.89;
    char *fStr = faasm::floatToStr(f);
    printf("f: %s\n", fStr);

    double d = 4.56;
    char *dStr = faasm::floatToStr(d);
    printf("d: %s\n", dStr);

    return 0;
}
