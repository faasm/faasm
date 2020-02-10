#include <iostream>
#include <omp.h>

void avg();
void avg_reduction();

int main() {
//    avg();
    avg_reduction();
    return 0;
}


void avg_round_robin() {
    int N = 600000000;
    double tavg = 0;

    double timer_start = omp_get_wtime();
    omp_set_num_threads(16);
#pragma omp parallel
    {
        double avg;
        int id = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        for (int i = id; i < N; i+=nthreads) {
            avg += i;
        }
#pragma omp atomic
        tavg += avg;
    }
    double timer_elapsed = omp_get_wtime() - timer_start;
    tavg = tavg / N;

    std::cout << tavg << " took " << timer_elapsed << std::endl;
    //     1 threads took 2.1
    //     4 threads took 0.7
    //    48 threads took 0.65
}

void  avg_reduction() {
    int N = 600000000;
    int j = 0;
    double tavg = 0;

    double timer_start = omp_get_wtime();
    omp_set_num_threads(48);

#pragma omp parallel for reduction(+:tavg)
    for (j = 0; j < N; ++j) {
        tavg += j;
    }

    double timer_elapsed = omp_get_wtime() - timer_start;
    tavg = tavg / N;

    std::cout << tavg << " took " << timer_elapsed << std::endl;
    //     1 threads took 2.1
    //     4 threads took 0.69
    //    48 threads took 0.65
}