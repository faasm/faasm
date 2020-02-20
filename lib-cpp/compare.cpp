#include "faasm/compare.h"

namespace faasm {
    bool compareIntArrays(int *a, int *b, int len) {
        for (int i = 0; i < len; i++) {
            if (a[i] != b[i]) {
                printf("Arrays not equal: A=");
                faasm::printArray(a, len);
                printf(" B=");
                faasm::printArray(b, len);
                printf("\n");

                return false;
            }
        }

        return true;
    }
}
