#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>

// I think flush is disappearing from OpenMP. Clang doesn't seem to care about argument lists
// and just implements it as a memory fence. So this example is probably no good to be honest. But
// at least it exists.
FAASM_MAIN_FUNC() {
    int data = 0, flag = 0;
    #pragma omp parallel num_threads(2) default(none) shared(data, flag)
    {
        if (omp_get_thread_num()==0) {
            /* Write to the data buffer that will be read by thread */
            data = 42;
            /* Flush data to thread 1 and strictly order the write to data
               relative to the write to the flag */
            #pragma omp flush(flag, data)
            /* Set flag to release thread 1 */
            flag = 1;
            /* Flush flag to ensure that thread 1 sees S-21 the change */
            #pragma omp flush(flag)
        }
        else if (omp_get_thread_num()==1) {
            /* Loop until we see the update to the flag */
            #pragma omp flush(flag, data)
            while (flag < 1) {
                #pragma omp flush(flag, data)
            }
            /* Values of flag and data are undefined */
            printf("flag=%d data=%d\n", flag, data);
            #pragma omp flush(flag, data)
            /* Values data will be 42, value of flag still undefined */
            printf("flag=%d data=%d\n", flag, data);
        }
    }
    return 0;
}
