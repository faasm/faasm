#include "faasm/faasm.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        printf("d: %li\n", sizeof(double));
        printf("f: %li\n", sizeof(float));
        printf("l: %li\n", sizeof(long));
        printf("i: %li\n", sizeof(int));
        printf("s: %li\n", sizeof(short));
        printf("c: %li\n", sizeof(char));
        printf("u: %li\n", sizeof(uint8_t));

        return 0;
    }
}
