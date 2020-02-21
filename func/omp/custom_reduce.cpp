#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>

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
    const int iterations = 50;
    Reducer sum(0);
    Reducer a[iterations];
    for (int i = 0; i < iterations; i++) {
        a[i].x = i;
    }

    #pragma omp parallel for num_threads(4) default(none) reduction(MyReducer:sum) shared(a)
    for (int i = 0; i < iterations; i++) {
        #pragma omp critical
        {
            printf("i=%d: thread #%d\n", i, omp_get_thread_num());
        }
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

