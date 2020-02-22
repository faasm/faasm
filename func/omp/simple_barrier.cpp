#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>

static int x1 = 0;
static int x2 = 0;
static int x3 = 0;
static int x4 = 0;
static int result1 = 0;
static int result2 = 0;
static int result3 = 0;
static int result4 = 0;

FAASM_MAIN_FUNC() {
    #pragma omp parallel num_threads(2) default(none) shared(x1, x2, x3, x4, result1, result2, result3, result4)
    {
        int threadNum = omp_get_thread_num();
        threadNum == 0 ? x1 = 5 : x2 = 5;

        #pragma omp barrier
        if (threadNum == 0) {
            result1 = x1 + x2;
        } else {
            result2 = x1 * x2;
        }

        threadNum == 0 ? x3 = 5 : x4 = 5;

        #pragma omp barrier
        if (threadNum == 0) {
            result3 = x3 + x4;
        } else {
            result4 = x3 * x4;
        }

    }

    if (result1 != 10 || result2 != 25) {
        printf("Barrier failed: result1: %d, result2: %d, x1: %d, x2: %d\n", result1, result2, x1, x2);
        return EXIT_FAILURE;
    }

    if (result3 != 10 || result4 != 25) {
        printf("Second Barrier failed: result3: %d, result4: %d, x3: %d, x4: %d\n", result3, result4, x3, x4);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
