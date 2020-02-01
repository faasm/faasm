#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>

#include "omp_helper.h"

bool failed = false;

FAASM_MAIN_FUNC() {
    const int total = 10;
    auto results = new AlignedElem[total];
    #pragma omp parallel for num_threads(2) schedule(static, 5) default(none) shared(results)
    for (int i = 0; i < total; i++) {
        results[i].i = i * (1 + omp_get_thread_num());
    }

    for (int i = 0; i < total; i++) {
        int j = i < 5? i : 2 * i;
        if (results[i].i != j) {
            printf("Element %i falsely set to %d, expected %d\n", i, results[i].i, j);
            failed = true;
        }
    }
    delete[] results;
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
