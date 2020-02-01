#include <omp.h>
#include <stdio.h>
#include <faasm/faasm.h>

bool failed = false;

FAASM_MAIN_FUNC() {
    int nThreads = 4;
    int iterations = 10;

    // Create zeroed counts
    auto counts = new int[nThreads];
    for(int i = 0; i < nThreads; i++) {
        counts[i] = 0;
    }

    #pragma omp parallel for num_threads(4) default(none) shared(counts, iterations)
    for (int i = 0; i < iterations; i++) {
        int threadNum = omp_get_thread_num();
        counts[threadNum]++;
    }

    for(int i = 0; i < nThreads; i++) {
        printf("%i = %i\n", i, counts[i]);
    }

    return 0;
} 