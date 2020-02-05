#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>


FAASM_MAIN_FUNC() {
    const int total = 10;
    auto results = new int[total];
    bool failed = false;

    #pragma omp parallel for num_threads(2) schedule(static, 5) default(none) shared(results)
    for (int i = 0; i < total; i++) {
        results[i] = i * (1 + omp_get_thread_num());
    }

    for (int i = 0; i < total; i++) {
        int expected = i < 5 ? i : 2 * i;
        if (results[i] != expected) {
            printf("Element %i falsely set to %d, expected %d\n", i, results[i], expected);
            failed = true;
        }
    }

    delete[] results;
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
