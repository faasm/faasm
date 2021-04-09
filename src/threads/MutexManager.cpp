#include <faabric/util/config.h>
#include <threads/MutexManager.h>

namespace threads {

void MutexManager::createMutex(int mutexId)
{
    mutexes[mutexId];
}

void MutexManager::lockMutex(int mutexId)
{
    getMutex(mutexId).lock();
}

void MutexManager::unlockMutex(int mutexId)
{
    getMutex(mutexId).unlock();
}

bool MutexManager::tryLockMutex(int mutexId)
{
    return getMutex(mutexId).try_lock();
}

void MutexManager::destroyMutex(int mutexId)
{
    mutexes.erase(mutexId);
}

std::mutex& MutexManager::getMutex(int mutexId)
{
    if (mutexes.count(mutexId) == 0) {
        throw std::runtime_error("Mutex does not exist");
    }

    return mutexes[mutexId];
}
}
