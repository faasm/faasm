#include <omp.h>
#include <stdio.h>
#include <faasm/faasm.h>

FAASM_MAIN_FUNC() {
    printf("Starting OpenMP\n");
#pragma omp parallel default(none)
    {
        int id = omp_get_thread_num();
        printf("OMP thread %d\n", id);
    };

    return 0;
}