#include <omp.h>
#include <cstdio>
#include <random>
#include "faasmp/reduction.h"

unsigned long thread_seed() {
    int threadNum = omp_get_thread_num();
    return static_cast<unsigned long>(threadNum * threadNum * 77 + 22 * threadNum + 1927);
}

int main(int argc, char **argv) {
    int iterations[3] = {4};
//    long iterations = 1;
    long num_threads = 1;
    long num_devices = -1;

    omp_set_default_device(num_devices);

    i64 result(90);
    #pragma omp parallel num_threads(num_threads) default(none) firstprivate(iterations) reduction(+:result)
    {
//        std::uniform_real_distribution<double> unif(0, 1);
//        std::mt19937_64 generator(thread_seed());
//        double x, y;
//
//        #pragma omp for nowait
//        for (long long i = 0; i < iterations[0]; ++i) {
//            x = unif(generator);
//            y = unif(generator);
//            if (x * x + y * y <= 1.0) {
//                ++result;
//            }
//        }
    }
//    printf("my coapy sscsaasnstructor %ld\n", (int64_t) result);

    double pi = (4.0 * (double) result) / iterations[0];

    if (abs(pi - 3.14) > 0.01) {
        printf("Low accuracy. Expected pi got %f\n", pi);
    }
}