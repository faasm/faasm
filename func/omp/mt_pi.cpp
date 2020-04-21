#include <omp.h>
#include <cstdio>
#include <random>

//#define num_devices num_threads

unsigned long thread_seed() {
    int threadNum = omp_get_thread_num();
    return threadNum * threadNum * 77 - 22 * threadNum + 1927;
}

int main(int argc, char **argv) {
    long long iterations = 1000LL;
    int num_threads = 1;
    if (argc == 3) {
        num_threads = std::stoi(argv[1]);
        iterations = std::stol(argv[2]);
    } else if (argc != 1) {
        printf("Usage: mt_pi [num_threads num_iterations]");
    }

//    omp_set_default_device(-2);

    long long result = 0;
    #pragma omp parallel num_threads(num_threads) default(none) firstprivate(iterations) reduction(+:result)
    {
        std::uniform_real_distribution<double> unif(0, 1);
        std::mt19937_64 generator(thread_seed());
        double x, y;
        #pragma omp for
        for (long i = 0; i < iterations; i++) {
            x = unif(generator);
            y = unif(generator);
            if (x * x + y * y <= 1.0) {
                result++;
            }
        }
    }

    double pi = (4.0 * result) / iterations;

    if (pi - 3.14 > 0.01) {
        printf("Low accuracy. Expected pi got %f\n", pi);
    }
    return EXIT_SUCCESS;
}