#include <cstdio>
#include <omp.h>

/*
 * Sums the numbers up to the total
 */
int getSum(int total)
{
    int sum = 0;
    for (int i = 0; i < total; i++) {
        sum += i;
    }
    return sum;
}

int main()
{
    int actual = 0;
    const int max = omp_get_max_threads();
    int expected = getSum(max);

#pragma omp parallel default(none) reduction(+ : actual)
    {
        actual = omp_get_thread_num();
    }

    if (actual != expected) {
        printf("Failed to set default num threads. Expected %d, got %d\n",
               expected,
               actual);
        return EXIT_FAILURE;
    }

    const int wanted = max - 1;
    // Need to run on multicore machine for rest of the test to be ok
    if (wanted == 0) {
        return EXIT_SUCCESS;
    }

    // Test setting with num_threads
    expected = getSum(wanted);
    actual = 0;
#pragma omp parallel num_threads(wanted) default(none) reduction(+ : actual)
    {
        actual = omp_get_thread_num();
    }

    if (actual != expected) {
        printf("Failed to set with pragma argument. Expected %d, got %d\n",
               expected,
               actual);
        return EXIT_FAILURE;
    }

    // Test value resets at parallel exit
    actual = 0;
    expected = getSum(max);
#pragma omp parallel default(none) reduction(+ : actual)
    {
        actual = omp_get_thread_num();
    }

    if (actual != expected) {
        printf("Failed to reset num threads. Expected %d, got %d\n",
               expected,
               actual);
        return EXIT_FAILURE;
    }

    // Test setting with omp_set
    omp_set_num_threads(wanted);
    expected = getSum(wanted);
    actual = 0;
#pragma omp parallel default(none) reduction(+ : actual)
    {
        actual = omp_get_thread_num();
    }

    if (actual != expected) {
        printf("Failed to set with omp_set_num_threads. Expected %d, got %d\n",
               expected,
               actual);
        return EXIT_FAILURE;
    }

    // Test set value is default even after parallel section
    if (wanted != omp_get_max_threads()) {
        printf("Max num threads changed. Expected %d, got %d\n",
               max,
               omp_get_max_threads());
        return EXIT_FAILURE;
    }

    actual = 0;
    expected = getSum(wanted);
#pragma omp parallel default(none) reduction(+ : actual)
    {
        actual = omp_get_thread_num();
    }

    if (actual != expected) {
        printf("Failed to keep set num threads. Expected %d, got %d\n",
               expected,
               actual);
        return EXIT_FAILURE;
    }

    // Test we can override set value with num_thread
    expected = getSum(max);
    actual = 0;
#pragma omp parallel num_threads(max) default(none) reduction(+ : actual)
    {
        actual = omp_get_thread_num();
    }

    if (actual != expected) {
        printf("Failed to override set value with pragma argument. Expected "
               "%d, got %d\n",
               expected,
               actual);
        return EXIT_FAILURE;
    }

    // We're done
    return EXIT_SUCCESS;
}
