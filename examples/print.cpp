#include "faasm.h"
#include "iostream"

namespace faasm {
    int exec(FaasmMemory *memory) {
        float d = 34.56;
        char buf[5];

        sprintf(buf, "%.2f", d);

        printf("f: %s\n", buf);

        return 0;
    }
}
