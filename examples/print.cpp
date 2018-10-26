#include "faasm.h"
#include "iostream"

namespace faasm {
    int exec(FaasmMemory *memory) {
        float d = 34.56;

        printf("f: %f\n", d);

        return 0;
    }
}
