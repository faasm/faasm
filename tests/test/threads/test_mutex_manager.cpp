#include <catch2/catch.hpp>

#include "utils.h"

#include <threads/MutexManager.h>

namespace tests {
TEST_CASE("Check lock/ unlock", "[threads]")
{
    cleanSystem();

    threads::MutexManager& tm = threads::getMutexManager();

    int idA = 123;
    int idB = 124;

    // Lock mutexes in main thread
    tm.lockMutex(idA);
    tm.lockMutex(idB);

    // Background threads that will try to lock and unlock the same mutexes
    std::thread tA([idA] {
        threads::MutexManager& tm = threads::getMutexManager();
        tm.lockMutex(idA);
        tm.unlockMutex(idA);
    });

    std::thread tB([idB] {
        threads::MutexManager& tm = threads::getMutexManager();
        tm.lockMutex(idB);
    });

    // Sleep to let things lock up
    usleep(500 * 1000);

    // Unlock the mutexes
    tm.unlockMutex(idA);
    tm.unlockMutex(idB);

    if (tA.joinable()) {
        tA.join();
    }
    if (tB.joinable()) {
        tB.join();
    }
}
}
