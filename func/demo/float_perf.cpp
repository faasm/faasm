#include "faasm/faasm.h"

#include "faasm/time.h"

void floatFunc(int n) {
    FAASM_PROF_START(floatMult)
    float f = 10000.0;
    for(int i = 0; i < n; i++) {
        if(i % 2 == 0) {
            f *= (float) i;
        }else {
            f /= (float) i;
        }
    }
    FAASM_PROF_END(floatMult)

    printf("Float value: %f\n", f);
}

void doubleFunc(int n) {
    FAASM_PROF_START(doubleMult)
    double d = 10000.0;
    for(int i = 0; i < n; i++) {
        if(i % 2 == 0) {
            d *= (double) i;
        }else {
            d /= (double) i;
        }
    }

    FAASM_PROF_END(doubleMult)

    printf("Double value: %f\n", d);
}

FAASM_MAIN_FUNC() {
    floatFunc(1000000);
    doubleFunc(1000000);

    return 0;
}