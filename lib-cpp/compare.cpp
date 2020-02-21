#include "faasm/compare.h"

namespace faasm {
    template<typename T>
    bool compareArrays(T *a, T *b, int len) {
        for (int i = 0; i < len; i++) {
            if (a[i] != b[i]) {
                printf("Arrays not equal: A=");
                faasm::printArray<T>(a, len);
                printf(" B=");
                faasm::printArray<T>(b, len);
                printf("\n");

                return false;
            }
        }

        return true;
    }

    template bool compareArrays<int>(int *a, int *b, int len);

    template bool compareArrays<double>(double *a, double *b, int len);
}
