#include "faasmp/reduction.h"
#include <cstdio>
#include <omp.h>
#include <random>

unsigned long thread_seed()
{
    int threadNum = omp_get_thread_num();
    return threadNum * threadNum * 77 - 22 * threadNum + 1927;
}

int main(int argc, char** argv)
{
    long long iterations = 1000L;
    long num_threads = 1;
    long num_devices = 0;
    if (argc == 4) {
        num_threads = std::stol(argv[1]);
        iterations = std::stoll(argv[2]);
        num_devices = std::stol(argv[3]);
    } else if (argc != 1) {
        printf("Usage: mt_pi [num_threads num_iterations num_devices]");
        return 5;
    }

    omp_set_num_threads(num_threads);
    omp_set_default_device(num_devices);

    i64 result(0);
#pragma omp parallel default(none) firstprivate(iterations) reduction(+:result)
    {
        std::uniform_real_distribution<double> unif(0, 1);
        std::mt19937_64 generator(thread_seed());
        double x, y;

#pragma omp for nowait
        for (std::int64_t i = 0; i < iterations; i++) {
            x = unif(generator);
            y = unif(generator);
            if (x * x + y * y <= 1.0) {
                ++result;
            }
        }
    }

    double pi = (4.0 * (double)result) / (iterations);

    if (pi - 3.14 > 0.01) {
        printf("Low accuracy. Expected pi got %f\n", pi);
    }
}