#include <omp.h>
#include <cstdio>

/*
int fun1() {
    int r = 0;
#pragma omp taskloop default(none) shared(r)
    for (int i = 0; i < 10; i++) {
        r += 1;
    }
    return r;
}
 */

int fun2() {
//    printf("fun2 num thread: %d, nested: %d\n", omp_get_num_threads(), omp_get_nested());
    int r = 0;
    #pragma omp parallel for num_threads(1) default(none) shared(r)
    for (int i = 0; i < 10; i++) {
//        printf("for fun2 thread #%d/%d, level %d, active level %d, team num %d, team size %d\n", omp_get_thread_num(), omp_get_num_threads(), omp_get_level(),
//               omp_get_active_level(), omp_get_team_num(), omp_get_team_size(omp_get_team_num()));
        r += 1;
    }
    return r;
}

int main() {
    #pragma omp parallel num_threads(1) default(none)
    {
//        printf("Original num thread: %d, nested: %d\n", omp_get_num_threads(), omp_get_nested());
        int local = 10;
        #pragma omp for nowait
        for (int i = 0; i < 100; i++) {
            local += 1;
        }
        //printf("Thread #%d: local is %d\n", omp_get_thread_num(), local);
//        printf("Main\n                     thread #%d/%d, level %d, active level %d, team num %d, team size %d\n", omp_get_thread_num(), omp_get_num_threads(), omp_get_level(),
//               omp_get_active_level(), omp_get_team_num(), omp_get_team_size(omp_get_team_num()));
        int mw = 0;
        #pragma omp for
        for (int i = 0; i < 10; i++) {
//            mw += fun1();
            mw += fun2();
        }
        printf("Thread #%d: mw is %d\n", omp_get_thread_num(), mw);
    }
}
