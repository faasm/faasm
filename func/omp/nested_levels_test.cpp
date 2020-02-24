/**
 *  Checks a few invariants of nested levels.
 *  The functions apply at the device level and the active level between the two is therefore independent.
 *  This example code does not test for multiple devices but rather look at reproducing the behaviour of clang which
 *  can be summed up that way: A level l is allowed parallelism if max_active_level >= l. The initial
 *  region level is 1, and max_active_level has LB 0 which means the first region can also be disabled that
 *  way and is not special in any aspect compared to the rest.
 */
#include <omp.h>
#include <cstdio>

bool level1_failed = false;
bool level2_failed = false;
bool level3_failed = false;

bool is_nested() {
    return omp_get_max_active_levels() > 1;
}

// Should run at 1 thread the first time (since upper level is running in
// parallel and nested isn't enabled), then in // with nesting enabled
int level3() {

    int previousLevel = omp_get_level();
    if (previousLevel != 2) {
        printf("Starting level 3: previous level failure, expected 2, got %d\n", previousLevel);
        level3_failed = true;
    }

    int r = 0;
    const int maxThreadForLevel = 3;
    int n; // actual number of threads that will run
    int nested = is_nested();
    if (nested) {
        n = maxThreadForLevel;
    } else {
        n = 1;
    }
    bool checkAllRun[maxThreadForLevel] = {false, false, false};

    #pragma omp parallel for num_threads(3) default(none) shared(r, n, checkAllRun, level3_failed)
    for (int i = 0; i < 3; i++) {
        int levelThreads = omp_get_num_threads();
        if (levelThreads != n) {
            printf("In level 3: failed get_num_threads, expected %d, got %d\n", n, levelThreads);
            level3_failed = true;
        }

        int threadNum = omp_get_thread_num();
        if (threadNum < 0 || n <= threadNum) {
            printf("In level 3: failed get_thread_num, expected something < %d, got %d\n", n, threadNum);
            level3_failed = true;
        }

        int currentLevel = omp_get_level();
        if (currentLevel!= 3) {
            printf("In level 3: level failure, expected 3, got %d\n", currentLevel);
            level3_failed = true;
        }

        checkAllRun[threadNum] = true; // mark this thread number as "has ran"

        #pragma omp atomic
        r += 3;
    }

    // Checks every thread has ran
    for (int i = 0; i < n; i++) {
        if (!checkAllRun[i]) {
            printf("In level 3: thread  %d didn't synchronise or start appropriately\n", i);
            level3_failed = true;
        }
    }

    return r;
}

// Should always be fully parallel: since the first section is initially
// sequential, and then allows for nested
int level2() {

    int previousLevel = omp_get_level();
    if (previousLevel != 1) {
        printf("Starting level 2: previous level failure, expected 1, got %d\n", previousLevel);
        level2_failed = true;
    }

    int r = 0;
    const int n = 2;
    const bool nested = is_nested(); // 0 then 1

    bool checkAllRun[n] = {false, false};

    #pragma omp parallel for num_threads(n) default(none) shared(r, checkAllRun, n, level2_failed)
    for (int i = 0; i < 2; i++) {

        if (nested != is_nested()) {
            printf("In level 2, did not inherit the nested value of the parent of %d\n", nested);
            level2_failed = true;
        }

        int levelThreads = omp_get_num_threads();
        if (levelThreads != n) {
            printf("In level 2: failed get_num_threads, expected %d, got %d with nested %d, upper nested %d\n", n, levelThreads, is_nested(), nested);
            level2_failed = true;
        }

        int threadNum = omp_get_thread_num();
        if (threadNum < 0 || n <= threadNum) {
            printf("In level 2: failed get_thread_num, expected something < %d, got %d\n", n, threadNum);
            level2_failed = true;
        }

        int currentLevel = omp_get_level();
        if (currentLevel!= 2) {
            printf("In level 3: level failure, expected 2, got %d\n", currentLevel);
            level2_failed = true;
        }

        checkAllRun[threadNum] = true;

        r += level3();
    }

    for (int i = 0; i < n; i++) {
        if (!checkAllRun[i]) {
            printf("In level 2: thread  %d didn't synchronise or start appropriately\n", i);
            level2_failed = true;
        }
    }

    return r;
}

int main() {
    int val1 = 0, val2 = 0, val3 = 0;
    int activeLevels;

    // Behaviour changed in newer versions of Clang with deprecation of omp_get_level
    // Default used to be MaxInt
    activeLevels = omp_get_max_active_levels();
    if (activeLevels != 1) {
        printf("Nested levels should be off by default, got %d\n", activeLevels);
        return EXIT_FAILURE;
    }

    omp_set_max_active_levels(0); // Essentially Disable OpenMP
    #pragma omp parallel num_threads(4) default(none) shared(val3)
    {
        #pragma omp atomic
        val3 += omp_get_thread_num();
    }
    if (val3 != 0) {
        printf("Nested levels of 0 means no parallel code\n");
        return EXIT_FAILURE;
    }

    omp_set_max_active_levels(100);
    omp_set_max_active_levels(-30);
    activeLevels = omp_get_max_active_levels();
    if (activeLevels != 100) {
        printf("Nested should ignore negative numbers\n");
        return EXIT_FAILURE;
    }

    // Disable nesting for first part of test
    omp_set_max_active_levels(1);

    #pragma omp parallel num_threads(1) default(none) shared(val1)
    {
        #pragma omp for
        for (int i = 0; i < 2; i++) {
            val1 += level2();
        }
    }

    // Useful to debug print statements before and after, enable nesting
    printf("--------- Enabling nesting---------\n");
    int manyLevels = 100;
    omp_set_max_active_levels(manyLevels);

    #pragma omp parallel num_threads(2) default(none) shared(val2)
    {
        #pragma omp for
        for (int i = 0; i < 2; i++) {
            #pragma omp atomic
            val2 += level2();
        }
    }

    if (val1 != val2) {
        printf("Something has gone wrong: val1 %d. val2 %d\n", val1, val2);
        level1_failed = true;
    }

    activeLevels = omp_get_max_active_levels();
    if (activeLevels != manyLevels) {
        printf("Nested levels should not have changed. Expected %d got %d\n", manyLevels, activeLevels);
        level1_failed = true;
    }
    return level1_failed || level2_failed || level3_failed;
}
