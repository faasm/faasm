#include <omp.h>
#include <stdio.h>
#include <faasm/faasm.h>

bool fail = false;
bool accessed = false; /* racy: should be atomic bool */

FAASM_MAIN_FUNC() {
    int main_number = omp_get_thread_num();
    #pragma omp parallel default(none) shared(main_number)
    {
        #pragma omp master
        {
            if (accessed) {
                printf("Master section was entered multiple times\n");
                fail = true;
            }
            accessed = true;
            int local_number = omp_get_thread_num();
            if (main_number != local_number) {
                printf("Master thread index changed from %d to %d\n", main_number, local_number);
                fail = true;
            }
        }
    }
    if (fail) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
