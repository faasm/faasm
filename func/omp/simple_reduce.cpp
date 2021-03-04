#include <cstdio>
#include <omp.h>

int main(int argc, char* argv[])
{
    int count = 0;

    // Note: Loop will be divided evenly among the threads
    int chunkSize = 20;
    int nThreads = 5;
    int loopSize = nThreads * chunkSize;
#pragma omp parallel for num_threads(5) default(none) shared(loopSize) reduction(+ : count)
    for (int i = 0; i < loopSize; i++) {
        count += (omp_get_thread_num() + 1);
    }

    int expected = 0;
    for(int i = 0; i < nThreads; i++) {
        expected += (i + 1) * chunkSize;
    }

    if (count != expected) {
        printf("Failed reduce: expected %d, got %d\n", expected, count);
        return 1;
    } else {
        printf("Reduce as expected: %d\n", count);
        return 0;
    }
}
