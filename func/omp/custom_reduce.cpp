#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>

#define ITERATIONS 50

struct Reducer {
    Reducer(int x = 0) : x(x) {}
    int x;
    Reducer &operator+=(const Reducer &other) {
        x += other.x;
        return *this;
    }
};

#pragma omp declare reduction(MyReducer: Reducer: omp_out += omp_in)

FAASM_MAIN_FUNC() {
    Reducer sum(0);
    Reducer a[ITERATIONS];
    for (int i = 0; i < ITERATIONS; i++) {
        a[i].x = i;
    }

    #pragma omp parallel for num_threads(4) default(none) reduction(MyReducer:sum) shared(a)
    for (int i = 0; i < ITERATIONS; i++) {
        Reducer val(i * 2 + a[i].x);
        sum += val;
    }

    const int expected = 3675;
    if (sum.x != expected) {
        printf("Custom reduction failed. Expected %d but got %d\n", expected, sum.x);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

