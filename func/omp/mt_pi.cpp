#include <cstdio>
#include <omp.h>
#include <random>
 
#define ITERATIONS 100000
#define N_THREADS 4

unsigned long threadSeed()
{
    int threadNum = omp_get_thread_num();
    return threadNum * threadNum * 77 - 22 * threadNum + 1927;
}

int main(int argc, char** argv)
{
    long result = 0;

#pragma omp parallel num_threads(N_THREADS) default(none) reduction(+:result)
    {
        std::uniform_real_distribution<double> unif(0, 1);
        std::mt19937_64 generator(threadSeed());

#pragma omp for
        for (int i = 0; i < ITERATIONS; i++) {
            double x = unif(generator);
            double y = unif(generator);

            if (x * x + y * y <= 1.0) {
                result++;
            }
        }
    }

    double pi = (4.0 * result) / ITERATIONS;

    if (abs(pi - 3.14) > 0.01) {
        printf("Low accuracy. Expected pi got %f\n", pi);
        return 1;
    } else {
        printf("Calculated Pi as %.10f\n", pi);
        return 0;
    }
}
