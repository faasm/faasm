#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main() {
    const int N = 4;
    int a[N];
    int i;
    printf("Begin teams with %i threads \n", N);

    #pragma omp target map(to: N) map(tofrom: a)
    #pragma omp teams num_teams(2) thread_limit(N/2) default(none) shared(a)
    #pragma omp distribute
    for (i = 0; i < N; i++) {
        a[i] = omp_get_team_num();
    }

    /*
     * The following are required on certain compilers apparently:
    #pragma omp end distribute
    #pragma omp end teams
    #pragma omp end target
    */

    // Expected result:
    // a[0] = 0, a[1] = 0, a[2] = 1, a[3] = 1,
    for (i = 0; i < N; i++) {
        printf("a[%i] = %i, ", i, a[i]);
    }
    printf("\n");
}
