#include <omp.h>
#include <cstdio>
#include <random>
#include "../../libs/faasmp/faasmp/faasmp.h"

#define num_devices num_threads

#ifdef WASM_COMPILATION
// implement Custom reduce operators
//#else
// default zero cost abstraction implementation (like struct and overloaded +)
#endif

constexpr int ITERATIONS = 10000;
constexpr int EXPECTED = ITERATIONS * (ITERATIONS - 1) / 2;

int main() {
    omp_set_default_device(-2);
    long result = 0;
    #pragma omp parallel for num_devices(30) default(none) reduction(+:result)
    for (int i = 0; i < ITERATIONS; i++) {
        result += i;
    }

    if (result != EXPECTED) {
        printf("Custom reduction failed. Expected %d but got %ld\n", EXPECTED, result);
        return EXIT_FAILURE;
    }
    printf("All happy\n");
    return EXIT_SUCCESS;
}

int test() {
    MultiHostSum *psum = new MultiHostSum();
    MultiHostSum sum = *psum;
//    omp_set_default_device(-2);
    long result;

    #pragma omp parallel for num_threads(4) default(none) reduction(MultiHostSum:sum)
    {
        //#pragma omp for
        for (int i = 0; i < ITERATIONS; i++) {
            sum += MultiHostSum(i); // should avoid constructor call, insert zero cost.
        }
//        // Implied barrier
//        #pragma omp master
//        {
//            result = sum.get();
//        }
    }

    result = sum.get();
    if (result != EXPECTED) {
        printf("Custom reduction failed. Expected %d but got %ld\n", EXPECTED, result);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int run() {
    int count = 0;
    #pragma omp parallel num_threads(4) default(none) reduction(+:count)
    {
        std::uniform_real_distribution<double> unif(0, 1);
        std::mt19937 generator((1 + omp_get_thread_num()) * (1 + omp_get_device_num()));
        double x, y;
        #pragma omp for
        for (int i = 0; i < ITERATIONS; i++) {
            x = unif(generator);
            y = unif(generator);
            if (x * x + y * y <= 1.0) {
                count++;
            }
        }
    }
    printf("Pi: %f\n", 4.0 * count / ITERATIONS); // 3.14159...

    return EXIT_SUCCESS;
}
