#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

namespace threads {

class MutexManager
{
  public:
    void clear();

    void createMutex(int mutexId);

    void lockMutex(int mutexId);

    void unlockMutex(int mutexId);

    bool tryLockMutex(int mutexId);

    void destroyMutex(int mutexId);

  private:
    std::unordered_map<int, std::mutex> mutexes;
};
}
