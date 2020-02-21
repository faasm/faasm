#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>
#include <random>

constexpr int ITERATIONS = 1000000000;

FAASM_MAIN_FUNC() {
    int count = 0;
    #pragma omp parallel num_threads(4) default(none) reduction(+:count)
    {
        std::uniform_real_distribution<double> unif(0, 1);
        std::mt19937 generator(omp_get_thread_num());
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

