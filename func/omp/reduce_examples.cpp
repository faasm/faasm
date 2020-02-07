#include <omp.h>
#include <cstdio>
#include <faasm/faasm.h>
#include <random>

constexpr int ITERATIONS = 400;

//asm(".data"
//    ".comm .gomp_critical_user_,32,8"
//    ".data"
//    "ALIGN 8"
//    ".global __kmp_unnamed_critical_addr"
//    "__kmp_unnamed_critical_addr:"
//    ".8byte .gomp_critical_user_"
//    ".type __kmp_unnamed_critical_addr,@object"
//    ".size __kmp_unnamed_critical_addr,8"
//);

FAASM_MAIN_FUNC() {
    int count = 0;
#pragma omp parallel for num_threads(4) default(none) reduction(+:count)
    for (int i = 0; i < ITERATIONS; i++) {
        count += omp_get_thread_num() + 1;
    }

    int expected = 1 * 100 + 2 * 100 + 3 * 100 + 4 * 100;
    if (count != expected) {
        printf("Failed reduce: expected %d, got %d\n", count, expected);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

