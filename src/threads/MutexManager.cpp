#include <faabric/util/config.h>
#include <threads/MutexManager.h>

namespace threads {

// ----------------------------------------
// Note - in a lot of places, things will call lockMutex or unlockMutex without
// explicitly creating the mutex up-front, therefore we have to allow them to be
// created on the fly.
// ----------------------------------------

void MutexManager::clear()
{
    mutexes.clear();
}

void MutexManager::createMutex(int mutexId)
{
    mutexes[mutexId];
}

void MutexManager::lockMutex(int mutexId)
{
    mutexes[mutexId].lock();
}

void MutexManager::unlockMutex(int mutexId)
{
    if (mutexes.count(mutexId) > 0) {
        mutexes[mutexId].unlock();
    }
}

bool MutexManager::tryLockMutex(int mutexId)
{
    return mutexes[mutexId].try_lock();
}

void MutexManager::destroyMutex(int mutexId)
{
    mutexes.erase(mutexId);
}
}
