#include <array>
#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>

#include "omp_helper.h"

/**
 * Simple counting test for static and static chunk scheduling
 */
FAASM_MAIN_FUNC() {
    bool failed = false;
    const int nThreads = 4;
    const int iterations = 10;

    // Count number of times threads are called in each iterations
    std::array<AlignedElem, nThreads> counts = {0, 0, 0, 0};
    std::array<AlignedElem, nThreads> expect = {3, 3, 3, 1}; // Based on native run

    #pragma omp parallel for schedule(static, 3) num_threads(nThreads) default(none) shared(counts, iterations)
    for (int i = 0; i < iterations; i++) {
        int threadNum = omp_get_thread_num();
        counts[threadNum].i++;
    }

    if (counts != expect) {
        printf("Chunked static: expected {%d, %d, %d, %d}, got {%d, %d, %d, %d}\n", expect[0].i,
               expect[1].i, expect[2].i, expect[3].i, counts[0].i, counts[1].i,
               counts[2].i, counts[3].i);
        failed = true;
    }

    counts = {0, 0, 0, 0};
    expect = {3, 3, 2, 2}; // Based on native behaviour
    #pragma omp parallel for schedule(static) num_threads(nThreads) default(none) shared(counts, iterations)
    for (int i = 0; i < iterations; i++) {
        int threadNum = omp_get_thread_num();
        counts[threadNum].i++;
    }

    if (counts != expect) {
        printf("No hint static: expected {%d, %d, %d, %d}, got {%d, %d, %d, %d}\n", expect[0].i,
               expect[1].i, expect[2].i, expect[3].i, counts[0].i, counts[1].i,
               counts[2].i, counts[3].i);
        failed = true;
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
