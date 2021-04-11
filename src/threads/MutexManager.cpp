#include <faabric/util/config.h>
#include <threads/MutexManager.h>

namespace threads {

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
