/**
 * Checks a few invariants of nested levels.
 *
 * The functions apply at the device level and the active level between the two
 * is therefore independent.
 *
 * This example code does not test for multiple devices but rather looks at
 * reproducing the behaviour of clang.
 *
 * Level L is allowed parallelism if max_active_level >= L.
 * The initial region level is 1, and max_active_level can be set to zero,
 * which means the first region can also be disabled.
 */
#include <cstdio>
#include <omp.h>

bool levelOneFailed = false;
bool levelTwoFailed = false;
bool levelThreeFailed = false;

bool isNested()
{
    return omp_get_max_active_levels() > 1;
}

// Should run at 1 thread the first time (since upper level is running in
// parallel and nested isn't enabled), then in parallel with nesting enabled
int levelThree()
{
    int previousLevel = omp_get_level();
    if (previousLevel != 2) {
        printf("Starting level 3: previous level failure, expected 2, got %d\n",
               previousLevel);
        levelThreeFailed = true;
    }

    int returnValue = 0;
    const int nParallel = 3;

    // Actual number of threads that will run
    int expectedThreads;
    int nested = isNested();
    if (nested) {
        expectedThreads = nParallel;
    } else {
        expectedThreads = 1;
    }

    bool runFlags[nParallel] = { false, false, false };

#pragma omp parallel for num_threads(nParallel) default(none)                  \
  shared(returnValue, expectedThreads, runFlags, levelThreeFailed)
    for (int i = 0; i < 3; i++) {
        int levelThreads = omp_get_num_threads();

        // Check we have the expected number of threads
        if (levelThreads != expectedThreads) {
            printf("In level 3: failed get_num_threads, expected %d, got %d\n",
                   expectedThreads,
                   levelThreads);
            levelThreeFailed = true;
        }

        int threadNum = omp_get_thread_num();
        if (threadNum < 0 || threadNum > expectedThreads) {
            printf("In level 3: failed get_thread_num, expected something < "
                   "%d, got %d\n",
                   expectedThreads,
                   threadNum);
            levelThreeFailed = true;
        }

        int currentLevel = omp_get_level();
        if (currentLevel != 3) {
            printf("In level 3: level failure, expected 3, got %d\n",
                   currentLevel);
            levelThreeFailed = true;
        }

        // Record that this threads was successful
        runFlags[threadNum] = true;

#pragma omp atomic
        returnValue += 3;
    }

    // Checks every thread has run
    for (int i = 0; i < expectedThreads; i++) {
        if (!runFlags[i]) {
            printf("In level 3: thread  %d didn't synchronise or start "
                   "appropriately\n",
                   i);
            levelThreeFailed = true;
        }
    }

    return returnValue;
}

// Should always be fully parallel: since the first section is initially
// sequential, and then allows for nested
int levelTwo()
{
    int previousLevel = omp_get_level();

    if (previousLevel != 1) {
        printf("Starting level 2: previous level failure, expected 1, got %d\n",
               previousLevel);
        levelTwoFailed = true;
    }

    int returnValue = 0;
    const int expectedThreads = 2;

    const bool nested = isNested();

    bool checkAllRun[expectedThreads] = { false, false };

#pragma omp parallel for num_threads(expectedThreads) default(none)            \
  shared(returnValue, checkAllRun, expectedThreads, levelTwoFailed, nested)
    for (int i = 0; i < 2; i++) {

        if (nested != isNested()) {
            printf("In level 2, did not inherit the nested value of the parent "
                   "of %d\n",
                   nested);
            levelTwoFailed = true;
        }

        int levelThreads = omp_get_num_threads();
        if (levelThreads != expectedThreads) {
            printf("In level 2: failed get_num_threads, expected %d, got %d "
                   "with nested %d, upper nested %d\n",
                   expectedThreads,
                   levelThreads,
                   isNested(),
                   nested);
            levelTwoFailed = true;
        }

        int threadNum = omp_get_thread_num();
        if (threadNum < 0 || expectedThreads <= threadNum) {
            printf("In level 2: failed get_thread_num, expected something < "
                   "%d, got %d\n",
                   expectedThreads,
                   threadNum);
            levelTwoFailed = true;
        }

        int currentLevel = omp_get_level();
        if (currentLevel != 2) {
            printf("In level 2: level failure, expected 2, got %d\n",
                   currentLevel);
            levelTwoFailed = true;
        }

        checkAllRun[threadNum] = true;

        // Run level three
        returnValue += levelThree();
    }

    for (int i = 0; i < expectedThreads; i++) {
        if (!checkAllRun[i]) {
            printf("In level 2: thread %d didn't synchronise or start "
                   "appropriately\n",
                   i);
            levelTwoFailed = true;
        }
    }

    return returnValue;
}

int main()
{
    int val1 = 0, val2 = 0, val3 = 0;
    int activeLevels;

    // Behaviour changed in newer versions of Clang with deprecation of
    // omp_get_level Default used to be MaxInt
    activeLevels = omp_get_max_active_levels();
    if (activeLevels != 1) {
        printf("Nested levels should be 1 by default, got %d\n",
               activeLevels);
        return EXIT_FAILURE;
    }

    // Disable parallel regions
    omp_set_max_active_levels(0);

#pragma omp parallel num_threads(4) default(none) shared(val3)
    {
#pragma omp atomic
        val3 += omp_get_thread_num();
    }

    if (val3 != 0) {
        printf("Value should still be zero, as parallel should be blocked\n");
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
            val1 += levelTwo();
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
            val2 += levelTwo();
        }
    }

    if (val1 != val2) {
        printf("Something has gone wrong: val1 %d. val2 %d\n", val1, val2);
        levelOneFailed = true;
    }

    activeLevels = omp_get_max_active_levels();
    if (activeLevels != manyLevels) {
        printf("Nested levels should not have changed. Expected %d got %d\n",
               manyLevels,
               activeLevels);
        levelOneFailed = true;
    }
    return levelOneFailed || levelTwoFailed || levelThreeFailed;
}
