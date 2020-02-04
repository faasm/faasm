#include <omp.h>
#include <stdio.h>
#include <faasm/faasm.h>

bool failed = false;

/**
 * Note - this function is designed to test basic OpenMP functionality.
 * The loop must include both globally and locally shared variables.
 */

FAASM_MAIN_FUNC() {
    int nThreads = omp_get_max_threads();
    if(nThreads < 2) {
        printf("Need 2 or more threads but got %i\n", nThreads);
        return 1;
    }

    int mainThreadNum = omp_get_thread_num();
    if(mainThreadNum != 0) {
        printf("Main thread is not zero\n");
        return 1;
    }

    bool *flags = new bool[nThreads];

    // Note the shared variables here are both local and global
    #pragma omp parallel default(none) shared(flags, failed, nThreads)
    {
        int thisThreadNum = omp_get_thread_num();
        flags[thisThreadNum] = true;

        unsigned int currentThreads = omp_get_num_threads();
        if(currentThreads != nThreads) {
            printf("Only running %i threads but expected %i\n", currentThreads, nThreads);
            failed = true;
        }
    }

    if(failed) {
        printf("Did not get expected number of threads in parallel section\n");
        return 1;
    }

    for(int i = 0; i < nThreads; i++) {
        if(!flags[i]) {
            printf("Did not get a true flag for thread %i\n", i);
            return 1;
        }
    }

    printf("OpenMP checks succeeded\n");

    delete[] flags;
    return 0;
}