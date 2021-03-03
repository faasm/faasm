#include <cstdio>
#include <omp.h>
#include <random>

unsigned long threadSeed()
{
    int threadNum = omp_get_thread_num();
    return threadNum * threadNum * 77 - 22 * threadNum + 1927;
}

int main(int argc, char** argv)
{
    long long iterations = 5000LL;
    int numThreads = 4;
    if (argc == 3) {
        numThreads = std::stoi(argv[1]);
        iterations = std::stoll(argv[2]);
    } else if (argc != 1) {
        printf("Usage: mt_pi [numThreads num_iterations]");
    }

    long long result = 0;
#pragma omp parallel num_threads(numThreads) default(none) firstprivate(iterations) reduction(+:result)
    {
        std::uniform_real_distribution<double> unif(0, 1);
        std::mt19937_64 generator(threadSeed());
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

    if (abs(pi - 3.14) > 0.01) {
        printf("Low accuracy. Expected pi got %f\n", pi);
        return 1;
    } else {
        printf("Calculated Pi as %.2f\n", pi);
        return 0;
    }
}
