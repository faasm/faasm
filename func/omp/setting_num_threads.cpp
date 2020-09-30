#include <cstdio>
#include <omp.h>

int get_expected_value(int total)
{
    int expected = 0;
    for (int i = 0; i < total; i++) {
        expected += i;
    }
    return expected;
}

int main()
{
    int actual = 0;
    const int max = omp_get_max_threads();
    int expected = get_expected_value(max);
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
    expected = get_expected_value(wanted);
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
    expected = get_expected_value(max);
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
    expected = get_expected_value(wanted);
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
    expected = get_expected_value(wanted);
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
    expected = get_expected_value(max);
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
