#include "faasm/faasm.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        short s = 8;
        printf("s: %i\n", s);

        int i = 7;
        printf("i: %i\n", i);

        float f = 7.89;
        printf("f: %.2f\n", f);

        double d = 4.56;
        printf("d: %.2f\n", d);

        return 0;
    }
}
