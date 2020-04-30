#include <omp.h>
#include <cstdio>

#define num_devices num_threads

constexpr int ITERATIONS = 10000;
constexpr int EXPECTED = ITERATIONS * (ITERATIONS - 1) / 2;

int main() {
    omp_set_default_device(-2);
    long result = 0;
    #pragma omp parallel for num_threads(30) default(none) reduction(+:result)
    for (int i = 0; i < ITERATIONS; i++) {
        result += i;
    }

    if (result != EXPECTED) {
        printf("Custom reduction failed. Expected %d but got %ld\n", EXPECTED, result);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
