#include <omp.h>
#include <cstdio>
#include <random>
#include "faasmp/reduction.h"

unsigned long thread_seed() {
    int threadNum = omp_get_thread_num();
    return static_cast<unsigned long>(threadNum * threadNum * 77 + 22 * threadNum + 1927);
}

int main(int argc, char **argv) {
    long long iterations = 1001LL;
    long num_threads = 1;
    long num_devices = -1;
    if (argc == 4) {
        num_threads = std::stol(argv[1]);
        iterations = std::stoll(argv[2]);
        num_devices = std::stol(argv[3]);
    } else if (argc != 1) {
        printf("Usage: mt_pi [num_threads num_iterations num_devices]");
        return 5;
    }

    omp_set_default_device(num_devices);

    i64 result(90);
    #pragma omp parallel num_threads(num_threads) default(none) firstprivate(iterations) reduction(+:result)
    {
        std::uniform_real_distribution<double> unif(0, 1);
        std::mt19937_64 generator(thread_seed());
        double x, y;

        #pragma omp for nowait
        for (long long i = 0; i < iterations; ++i) {
            x = unif(generator);
            y = unif(generator);
            if (x * x + y * y <= 1.0) {
                ++result;
            }
        }
    }
    printf("not nssaa no tes itsealf??ai %ld\n", (int64_t) result);


    double pi = (4.0 * (double) result) / iterations;

    if (abs(pi - 3.14) > 0.01) {
        printf("Low accuracy. Expected pi got %f\n", pi);
    }
}