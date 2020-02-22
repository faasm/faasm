#include <omp.h>
#include <cstdio>

int main() {
    int total = 0;
    int expected = 12;

    omp_set_max_active_levels(2);

    #pragma omp parallel num_threads(2) default(none) shared(total)
    {
        #pragma omp critical
        {
            total += 1;
        }
        #pragma omp parallel num_threads(5) default(none) shared(total)
        {
            #pragma omp critical
            {
                total += 1;
            }
        }

    }
    if (total != expected) {
        printf("Bad nested parallel sections. Expected %d, got %d\n", expected, total);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
