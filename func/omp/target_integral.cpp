#include <omp.h>
#include <cstdio>

const long num_steps = 100000000;
const double pi = 3.1415;
const double epsilon = 0.0001;

int main() {
    double x, sum = 0.0;
    double step = 1.0 / (double) num_steps;
    int *threads = new int[omp_get_max_threads()];
    int run_once = 0;

    #pragma omp target map(sum)
    #pragma omp teams distribute parallel for default(none) private(x) firstprivate(step, run_once) shared(threads) reduction(+:sum)
    for (long i = 1; i <= num_steps; i++) {
        if (run_once == 0) { // There are more idiomatic ways to do this
            threads[omp_get_thread_num()] = omp_get_thread_num();
            run_once = 1;
        }
        x = (i - 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    for (int i = 0; i < omp_get_max_threads(); i++) {
        if (i != threads[i]) {
            printf("Thread %d didn't take part in computation\n ", i);
            delete[] threads;
            return EXIT_FAILURE;
        }
    }
    delete[] threads;

    double result = step * sum;
    if (abs(result - pi) > epsilon) {
        printf("Integral calculation failed. Expected %lf, got %lf\n ", pi, result);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
