#include <omp.h>
#include <stdio.h>
#include <faasm/faasm.h>

bool failed = false;
#define ITERATIONS 1

// Use base values else than 0 or 1 because they are often reset to that
#define BASE_VALUE 100

FAASM_MAIN_FUNC() {
    int var = BASE_VALUE + ITERATIONS;

    #pragma omp parallel for num_threads(1) default(none) shared(var)
    for (int i = 0; i < ITERATIONS; i++) {
        var--;
    }

    if (var != BASE_VALUE) {
        printf("Incorrect stack variable: actual %d, expected %d\n", var, BASE_VALUE);
        failed = true;
    }
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}