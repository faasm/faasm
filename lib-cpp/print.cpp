#include "faasm/print.h"

#include <stdio.h>

namespace faasm {
    /**
     * Horrible hack to get around a bug in the wasm backend which breaks printing of floats
     */
    char *floatToStr(float f) {
        // Print to 3 decimal places
        int lhs = int(f);

        int rhs = int(1000 * (f - lhs));

        // Hack because abs() doesn't compile
        if (rhs < 0) {
            rhs *= -1;
        }

        char *res = new char[15];
        sprintf(res, "%i.%i", lhs, rhs);

        return res;
    }

    void printArray(int *data, size_t size) {
        printf("[");
        for (size_t i = 0; i < size; i++) {
            printf("%i", data[i]);
            if (i < size - 1) {
                printf(",");
            }
        }
        printf("]");
    }
}
