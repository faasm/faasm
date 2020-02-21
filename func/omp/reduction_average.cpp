#include <iostream>
#include <omp.h>
#include <faasm/faasm.h>

double avg_round_robin();
double avg_reduction();

bool checkVal(const char *string, double val);

const int N = 600000000;
const double expected = 299999999.5;

FAASM_MAIN_FUNC() {
    bool failed = false;
    failed |= checkVal("Round robin with atomics", avg_round_robin());
    failed |= checkVal("Reduction construct", avg_reduction());
    if (failed) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

bool checkVal(const char *fun, double val) {
    if (expected - val >= 0.000001) {
        printf("Function %s failed: Expected %f but got %f \n", fun, expected, val);
        return true;
    }
    return false;
}

double avg_round_robin() {
    double tavg = 0;

//    double timer_start = omp_get_wtime();
    omp_set_num_threads(10);
    #pragma omp parallel default(none) shared(tavg)
    {
        double avg;
        int id = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        for (int i = id; i < N; i += nthreads) {
            avg += i;
        }
        #pragma omp atomic
        tavg += avg;
    }
//    double timer_elapsed = omp_get_wtime() - timer_start;
    tavg = tavg / N;
    return tavg;

//    std::cout << tavg << " took " << timer_elapsed << std::endl;
    //     1 threads took 2.1
    //     4 threads took 0.7
    //    48 threads took 0.65
}

double avg_reduction() {
    double tavg = 0;

//    double timer_start = omp_get_wtime();
    omp_set_num_threads(48);
    #pragma omp parallel for default(none) reduction(+:tavg) shared(N)
    for (int j = 0; j < N; ++j) {
        tavg += j;
    }

//    double timer_elapsed = omp_get_wtime() - timer_start;
    tavg = tavg / N;
    return tavg;

//    std::cout << tavg << " took " << timer_elapsed << std::endl;
    //     1 threads took 2.1
    //     4 threads took 0.69
    //    48 threads took 0.65
}