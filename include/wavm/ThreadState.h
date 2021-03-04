#pragma once

#include <mutex>
#include <vector>

#include <proto/faabric.pb.h>

#include <faabric/util/barrier.h>
#include <faabric/util/environment.h>
#include <faabric/util/locks.h>

namespace wasm {

// A Level is a layer of threads in an OpenMP application.
// Note, defaults are set to mimic Clang 9.0.1 behaviour
class Level
{
  public:
    // Number of nested OpenMP constructs
    const int depth = 0;

    // Number of parallel regions with more than 1 thread above this level
    const int activeLevels = 0;

    // Max number of effective parallel regions allowed from the top
    int maxActiveLevels = 1;

    // Number of threads of this level
    const int numThreads = 1;

    // Desired number of thread set by omp_set_num_threads for all future levels
    int wantedThreads = -1;

    // Num threads pushed by compiler, valid for one parallel section.
    // Overrides wantedThreads
    int pushedThreads = -1;

    // Barrier for synchronization
    faabric::util::Barrier barrier;

    // Mutex used for reductions and critical sections
    std::recursive_mutex levelMutex;

    // Condition variable and count used for nowaits
    int nowaitCount = 0;
    std::mutex nowaitMutex;
    std::condition_variable nowaitCv;

    // Local constructor
    Level(const std::shared_ptr<Level>& parent, int numThreadsIn);

    // Distributed constructor
    Level(int depth,
          int activeLevelsIn,
          int maxActiveLevelsIn,
          int numThreadsIn);

    int getMaxThreadsAtNextLevel() const;

    void masterWait(int threadNum);
};

class OpenMPContext
{
  public:
    int threadNumber = -1;

    std::shared_ptr<Level> level = nullptr;
};

OpenMPContext& getOpenMPContext();

void setUpOpenMPContext(const faabric::Message& msg);

void setUpOpenMPContext(const int threadId, std::shared_ptr<Level> &level);

}
