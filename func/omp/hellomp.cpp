#include <omp.h>
#include <stdio.h>

int main() {
#pragma omp parallel default(none)
    {
        int id = omp_get_thread_num();
        printf("OMP thread %d\n", id);
    };
}