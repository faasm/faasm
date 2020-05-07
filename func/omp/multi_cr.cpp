#include <omp.h>
#include <cstdio>
#include <random>
#include "faasmp/reduction.h"

unsigned long thread_seed() {
    int threadNum = omp_get_thread_num();
    return static_cast<unsigned long>(threadNum * threadNum * 77 + 22 * threadNum + 1927);
}

constexpr int32_t iterations = 1'000;

int main(int argc, char **argv) {
    uint32_t num_threads = 1;
    int32_t num_devices = 0;
    uint32_t num_times = 1;
    if (argc == 4) {
        num_threads = std::stoul(argv[1]);
        num_times = std::stoul(argv[2]);
        num_devices = std::stol(argv[3]);
    } else if (argc != 1) {
        printf("Usage: multi_cr [num_threads num_times num_devices]");
        return 5;
    }

    // I'm boolean impaired, this is bad even for me writing this, sorry :)
    if (num_times == 0 || num_threads == 0 || (num_devices != 0 && num_devices != -1)) {
        printf("Bad args value");
        return 6;
    }

    omp_set_default_device(num_devices);

    std::vector<double> pis;
    pis.reserve(num_times);
    for (int i = 0; i < num_times; i++) {
        i64 result(0);
        #pragma omp parallel num_threads(num_threads) default(none) firstprivate(iterations) reduction(+:result)
        {
            std::uniform_real_distribution<double> unif(0, 1);
            std::mt19937_64 generator(thread_seed());
            double x, y;

            #pragma omp for nowait
            for (int32_t j = 0; j < iterations; ++j) {
                x = unif(generator);
                y = unif(generator);
                if (x * x + y * y <= 1.0) {
                    ++result;
                }
            }
        }
        pis.emplace_back((4.0 * (double) result) / iterations);
    }

    double reference = pis[0];
    if (abs(reference - 3.14) > 0.01) {
        printf("Low accuracy Expected pi got %f\n", reference);
    }

    for (double pi : pis) {
        if (pi != reference) {
            printf("Expected %f, got %f\n", reference, pi);
            break;
        }
    }
}