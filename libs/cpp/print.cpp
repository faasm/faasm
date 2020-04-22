#include "faasm/print.h"

#include <stdio.h>
#include <cstdint>

namespace faasm {
    template <typename T>
    void printArray(T * data, size_t size);

    template<> void printArray<int>(int *data, size_t size) {
        printf("[");
        for (size_t i = 0; i < size; i++) {
            printf("%i", data[i]);
            if (i < size - 1) {
                printf(", ");
            }
        }
        printf("]");
    }

    template<> void printArray<uint8_t>(uint8_t *data, size_t size) {
        printf("[");
        for (size_t i = 0; i < size; i++) {
            printf("%i", data[i]);
            if (i < size - 1) {
                printf(", ");
            }
        }
        printf("]");
    }

    template<> void printArray<double>(double *data, size_t size) {
        printf("[");
        for (size_t i = 0; i < size; i++) {
            printf("%f", data[i]);
            if (i < size - 1) {
                printf(", ");
            }
        }
        printf("]");
    }
}
