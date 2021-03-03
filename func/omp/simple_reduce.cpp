#include <cstdio>
#include <faasm/faasm.h>
#include <omp.h>

int main(int argc, char* argv[])
{
    int count = 0;

#pragma omp parallel for num_threads(5) default(none) reduction(+ : count)
    for (int i = 0; i < 100; i++) {
        count += (omp_get_thread_num() + 1);
    }

    int expected = 1 * 100 + 2 * 100 + 3 * 100 + 4 * 100 + 5 * 100;

    if (count != expected) {
        printf("Failed reduce: expected %d, got %d\n", expected, count);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
