#include <cstdio>
#include <faasm/faasm.h>
#include <omp.h>

int main(int argc, char* argv[])
{
    const int expected = 1;
    int mine = expected; // Only modified in critical section
    bool failed = false;

    int veryRandom = 2;
#pragma omp parallel for num_threads(4) default(none)                          \
  shared(veryRandom, mine, expected, failed)
    for (int i = 0; i < 1000; i++) {
#pragma omp critical
        {
            if (mine != expected) {
                printf("Mine is %d, thread %d\n", mine, omp_get_thread_num());
                failed = true;
            }
            mine = 2;
            for (int j = 0; j < 10000; j++) {
                mine = (mine + veryRandom) % 4 + 10;
            }
            // sets back to original value
            mine = expected;
        }
        veryRandom = (omp_get_thread_num() + 1 * veryRandom) % 7;
    }

    if (failed || mine != expected) {
        printf("Critical section error, assumption failed: %d\n", mine);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
