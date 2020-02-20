#include <omp.h>
#include<cstdio>
#include<faasm/faasm.h>

double integral_atomic();
double integral_roundrobin();
double integral_reduction();
double integral_better_reduction();
bool checkResult(const char *func, double reduction);

FAASM_MAIN_FUNC() {
    bool failed = false;
    failed |= checkResult("Atomic", integral_atomic());
    failed |= checkResult("RR", integral_roundrobin());
    failed |= checkResult("Reduction", integral_reduction());
    failed |= checkResult("Better reduction", integral_better_reduction());
    if (failed) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

bool checkResult(const char *func, double reduction) {
    if (reduction < 3 || reduction > 4) {
        printf("Function %s didn't return pi but %f\n", func, reduction);
        return true;
    }
    return false;
}

double omp_get_wtime() {
    // TODO - actually return time
    return 1.0;
}

double integral_roundrobin() {
    int NTHREADS = 48, nthreads;
    long num_steps = 100000000;
    double step = 0;
    double pi = 0.0;
    double sum[NTHREADS];

    step = 1.0 / (double) num_steps;
    double timer_start = omp_get_wtime();
    omp_set_num_threads(NTHREADS);

    #pragma omp parallel default(none) shared(nthreads, num_steps, step, sum)
    {
        int i, id, lnthreads;
        double x;

        lnthreads = omp_get_num_threads();
        id = omp_get_thread_num();
        if (id == 0)
            nthreads = lnthreads;

        for (i = id, sum[id] = 0; i < num_steps; i += lnthreads) {
            x = (i + 0.5) * step;
            sum[id] += 4.0 / (1.0 + x * x);
        }

    }
    for (int i = 0; i < nthreads; ++i) {
        pi += sum[i] * step;
    }

    double timer_took = omp_get_wtime() - timer_start;
    printf("RR took %f, pi: %f\n", timer_took, pi);
    return pi;
    // 1 threads  --> 0.57 seconds.
    // 4 threads  --> 1.34 seconds.
    // 24 threads --> 0.59 seconds.
    // 48 threads --> 0.46 seconds.
}


double integral_atomic() {
    int NTHREADS = 4;
    long num_steps = 100000000;
    double step = 0;
    double pi = 0.0;

    step = 1.0 / (double) num_steps;
    double timer_start = omp_get_wtime();
    omp_set_num_threads(NTHREADS);

    #pragma omp parallel default(none) shared(num_steps, step, pi)
    {
        int i, id, lnthreads;
        double x, sum = 0;

        lnthreads = omp_get_num_threads();
        id = omp_get_thread_num();

        for (i = id; i < num_steps; i += lnthreads) {
            x = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }

        #pragma omp atomic
        pi += sum * step;

    }

    double timer_took = omp_get_wtime() - timer_start;
    printf("Atomic took %f, pi: %f\n", timer_took, pi);
    return pi;
    // 1 threads  --> 0.53 seconds.
    // 4 threads  --> 0.25 seconds.
    // 24 threads --> 0.24 seconds.
    // 48 threads --> 0.21 seconds.
}


double integral_reduction() {
    int NTHREADS = 48;
    long num_steps = 100000000;
    double step = 0;
    double pi = 0.0;
    double sum = 0;
    int i = 0;

    step = 1.0 / (double) num_steps;

    omp_set_num_threads(NTHREADS);
    double timer_start = omp_get_wtime();

    #pragma omp parallel for default(none) shared(num_steps, step) reduction(+:sum)
    for (i = 0; i < num_steps; ++i) {
        int x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    pi = sum * step;

    double timer_took = omp_get_wtime() - timer_start;
    printf("Reduction took %f, pi: %f\n", timer_took, pi);
    return pi;
    // 1 threads  --> 0.55 seconds.
    // 4 threads  --> 0.24 seconds.
    // 24 threads --> 0.24 seconds.
    // 48 threads --> 0.23 seconds.
}

double integral_better_reduction() {
    // this version is better because it can work in the case of non-threaded environments.
    int NTHREADS = 48;
    long num_steps = 100000000;
    double step = 0;
    double pi = 0.0;
    double sum = 0;
    int i = 0, x;

    step = 1.0 / (double) num_steps;

    omp_set_num_threads(NTHREADS);
    double timer_start = omp_get_wtime();

    #pragma omp parallel for private(x) default(none) shared(num_steps, step) reduction(+:sum)
    for (i = 0; i < num_steps; ++i) {
        x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    pi = sum * step;

    double timer_took = omp_get_wtime() - timer_start;
    printf("Better reduction took %f, pi: %f\n", timer_took, pi);
    return pi;
    // 1 threads  --> 0.55 seconds.
    // 4 threads  --> 0.24 seconds.
    // 24 threads --> 0.24 seconds.
    // 48 threads --> 0.23 seconds.
}