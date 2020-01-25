#include <omp.h>
#include <stdio.h>

void printThreadInfo(const char* label) {
    int thisThread = omp_get_thread_num();
    unsigned int totalThreads = omp_get_num_threads();
    printf("%s: %d of %d\n", label, thisThread, totalThreads);
}

// Creates parallel section with set number of threads
int main() {
    printThreadInfo("Main before");

    #pragma omp parallel default(none) num_threads(2)
    {
        printThreadInfo("See me twice");
    }

    printThreadInfo("Main after");
    return 0;
}
