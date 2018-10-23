#ifndef FAASM_RANDOM_H
#define FAASM_RANDOM_H

#include <stdlib.h>

namespace faasm {
    void shuffle(long *array, size_t nElems) {
        for (size_t i = 0; i < nElems - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (nElems - i) + 1);

            // Shuffle the elements
            long t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

#endif
