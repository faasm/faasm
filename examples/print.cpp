#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        float f = 7.89;
        printf("f: %f\n", f);

        double d = 4.56;
        printf("d: %f\n", d);

        int i = 7;
        printf("i: %i\n", i);

        short s = 8;
        printf("s: %i\n", s);

        return 0;
    }
}
