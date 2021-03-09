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

bool checkLevel(int expectedLevel,
                int expectedThreads,
                int expectedMaxActiveLevels)
{
    bool failed = false;

    int actualLevel = omp_get_level();
    int actualThreads = omp_get_num_threads();
    int actualMaxActiveLevels = omp_get_max_active_levels();

    if (actualLevel != expectedLevel) {
        printf("Level not as expected, %i != %i\n", actualLevel, expectedLevel);
        failed |= true;
    }

    if (actualThreads != expectedThreads) {
        printf("Threads not as expected, %i != %i\n",
               actualThreads,
               expectedThreads);
        failed |= true;
    }

    if (actualMaxActiveLevels != expectedMaxActiveLevels) {
        printf("Max active levels not as expected, %i != %i\n",
               actualMaxActiveLevels,
               expectedMaxActiveLevels);
        failed |= true;
    }

    return failed;
}

int main()
{
    int activeLevels = omp_get_max_active_levels();
    if (activeLevels != 1) {
        printf("Max active levels should be 1 by default, got %d\n",
               activeLevels);
        return 1;
    }

    bool failed = false;

    // ----------------------------------
    // Check parallelism can be switched off
    // ----------------------------------
    printf("------ ONE LEVEL ------\n");
    omp_set_max_active_levels(0);

    int baseLevelCount = 0;
#pragma omp parallel num_threads(4) default(none) shared(baseLevelCount, failed)
    {
        failed |= checkLevel(1, 1, 0);
#pragma omp atomic
        baseLevelCount++;
    }

    if (failed) {
        printf("Level check failed on no parallelism\n");
        return 1;
    }

    if (baseLevelCount != 1) {
        printf("Count is %i but should be one. Should only be one thread\n",
               baseLevelCount);
        return 1;
    }

    // ----------------------------------
    // Check two levels of parallelism
    // ----------------------------------
    printf("------ TWO LEVELS ------\n");
    omp_set_max_active_levels(1);
    baseLevelCount = 0;
    int nestedLevelCount = 0;

#pragma omp parallel num_threads(4) default(none)                              \
  shared(baseLevelCount, nestedLevelCount, failed)
    // Base level
    {
        failed |= checkLevel(1, 4, 1);
#pragma omp atomic
        baseLevelCount++;

#pragma omp parallel num_threads(4) default(none)                              \
  shared(nestedLevelCount, failed)
        {
            // Nested level
            // Number of threads should be forced to one (note that there will
            // be one of these levels for each of the parent threads)
            failed |= checkLevel(2, 1, 1);
#pragma omp atomic
            nestedLevelCount++;
        }
    }

    if (baseLevelCount != 4) {
        printf("Expected base count to be 4 but was %i\n", baseLevelCount);
        return 1;
    }

    if (nestedLevelCount != 4) {
        printf("Expected nested count to be 4 but was %i\n", nestedLevelCount);
        return 1;
    }

    // ----------------------------------
    // Check several levels of parallelism
    // ----------------------------------
    printf("------ THREE LEVEL ------\n");
    // Experiment changing levels
    omp_set_max_active_levels(100);
    omp_set_max_active_levels(-30);

    activeLevels = omp_get_max_active_levels();
    if (activeLevels != 100) {
        printf("Nested should ignore negative numbers\n");
        return EXIT_FAILURE;
    }

    baseLevelCount = 0;
    nestedLevelCount = 0;
    int doubleNestedLevelCount = 0;

#pragma omp parallel num_threads(2) default(none)                              \
  shared(baseLevelCount, nestedLevelCount, doubleNestedLevelCount, failed)
    // Base level
    {
        failed |= checkLevel(1, 2, 100);

#pragma omp atomic
        baseLevelCount++;

#pragma omp parallel num_threads(3) default(none)                              \
  shared(nestedLevelCount, doubleNestedLevelCount, failed)
        // Nested block
        {
            failed |= checkLevel(2, 3, 100);
#pragma omp atomic
            nestedLevelCount++;

#pragma omp parallel num_threads(4) default(none)                              \
  shared(doubleNestedLevelCount, failed)
            // Doubly nested block
            {
                failed |= checkLevel(3, 4, 100);

#pragma omp atomic
                doubleNestedLevelCount++;
            }
        }
    }

    if (baseLevelCount != 2) {
        printf("Expected base count to be 2 but was %i\n", baseLevelCount);
        return 1;
    }

    if (nestedLevelCount != 6) {
        printf("Expected nested count to be 6 but was %i\n", nestedLevelCount);
        return 1;
    }

    if (doubleNestedLevelCount != 24) {
        printf("Expected double nested count to be 24 but was %i\n",
               doubleNestedLevelCount);
        return 1;
    }

    return 0;
}
