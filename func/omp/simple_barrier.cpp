#include <omp.h>
#include <stdio.h>
#include <faasm/faasm.h>

static int x1 = 0;
static int x2 = 0;
static int result1 = 0;
static int result2 = 0;

FAASM_MAIN_FUNC() {
    #pragma omp parallel num_threads(2) default(none) shared(x1, x2, result1, result2)
    {
        int threadNum = omp_get_thread_num();
        threadNum == 0 ? x1 = 5 : x2 = 5;

        #pragma omp barrier
        threadNum == 0 ? result1 = x1 + x2 : result2 = x1 * x2;
    }
    if (result1 != 10 || result2 != 25) {
        printf("Barrier failed: result1: %d, result2: %d, x1: %d, x2: %d\n", result1, result2, x1, x2);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
