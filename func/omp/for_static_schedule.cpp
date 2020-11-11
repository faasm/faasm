#include <cstdio>
#include <omp.h>

#include <faasm/faasm.h>

#define ITERATIONS 10

bool compareArrays(const char* label, int* actual, int* expected)
{
    for (size_t i = 0; i < 4; i++) {
        if (actual[i] != expected[i]) {
            printf(
              "%s failed: expected {%d, %d, %d, %d}, got {%d, %d, %d, %d}\n",
              label,
              expected[0],
              expected[1],
              expected[2],
              expected[3],
              actual[0],
              actual[1],
              actual[2],
              actual[3]);

            return false;
        }
    }

    return true;
}

/**
 * Simple counting test for static and static chunk scheduling
 */
int main(int argc, char* argv[])
{
    int countsA[4] = { 0, 0, 0, 0 };
    int expectedA[4] = { 3, 3, 3, 1 }; // Based on native behaviour

#pragma omp parallel for schedule(static, 3) num_threads(4) default(none)      \
  shared(countsA)
    for (int i = 0; i < ITERATIONS; i++) {
        int threadNum = omp_get_thread_num();
        countsA[threadNum]++;
    }

    if (!compareArrays("Static with hint", countsA, expectedA)) {
        return 1;
    }

    int countsB[4] = { 0, 0, 0, 0 };
    int expectedB[4] = { 3, 3, 2, 2 }; // Based on native behaviour

#pragma omp parallel for schedule(static) num_threads(4) default(none)         \
  shared(countsB)
    for (int i = 0; i < ITERATIONS; i++) {
        int threadNum = omp_get_thread_num();
        countsB[threadNum]++;
    }

    if (!compareArrays("No hint static", countsB, expectedB)) {
        return 1;
    }

    return 0;
}
