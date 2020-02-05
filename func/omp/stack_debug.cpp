#include <omp.h>
#include <stdio.h>
#include <faasm/faasm.h>
#include <faasmp/faasmp.h>


FAASM_MAIN_FUNC() {

    bool failed = false;

    #pragma omp parallel default(none) shared(failed)
    {
        int a = omp_get_thread_num();
        int b = -1;

        __faasmp_debug_copy(&a, &b);

        if (b == -1) {
            printf("Write failed (b == 0)\n");
            failed = true;
        } else if (b != a) {
            printf("Something else wrote to b (b == %i)\n", b);
            failed = true;
        } else if (b == a) {
            printf("Successful write to b (b == %i)\n", b);
        } else {
            printf("b had some unexpected value (b == %i)\n", b);
            failed = true;
        }
    }

    return failed ? 1 : 0;
}