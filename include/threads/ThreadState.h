#pragma once

#include <future>
#include <mutex>
#include <vector>

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/barrier.h>
#include <faabric/util/environment.h>
#include <faabric/util/locks.h>

namespace threads {

// A Level is a layer of threads in an OpenMP application.
// Note, defaults are set to mimic Clang 9.0.1 behaviour
class Level
{
  public:
    // Number of nested OpenMP constructs
    int depth = 0;

    // Number of parallel regions with more than 1 thread above this level
    int activeLevels = 0;

    // Max number of active parallel regions allowed
    int maxActiveLevels = 1;

    // Number of threads of this level
    int numThreads = 1;

    // Desired number of thread set by omp_set_num_threads for all future levels
    int wantedThreads = -1;

    // Num threads pushed by compiler, valid for one parallel section.
    // Overrides wantedThreads
    int pushedThreads = -1;

    // Barrier for synchronization
    faabric::util::Barrier barrier;

    // Mutex used for reductions and critical sections
    std::recursive_mutex levelMutex;

    Level(int numThreadsIn);

    void fromParentLevel(const std::shared_ptr<Level>& parent);

    // Instance functions
    int getMaxThreadsAtNextLevel() const;

    void masterWait(int threadNum);

  private:
    // Condition variable and count used for nowaits
    int nowaitCount = 0;
    std::mutex nowaitMutex;
    std::condition_variable nowaitCv;
};

class OpenMPTask
{
  public:
    OpenMPTask(faabric::Message* parentMsgIn,
               faabric::Message& msgIn,
               std::shared_ptr<threads::Level> nextLevelIn,
               int threadIdxIn)
      : parentMsg(parentMsgIn)
      , msg(msgIn)
      , nextLevel(nextLevelIn)
      , threadIdx(threadIdxIn)
    {}

    faabric::Message* parentMsg;
    faabric::Message& msg;
    std::shared_ptr<threads::Level> nextLevel;
    int threadIdx;
};

class OpenMPContext
{
  public:
    int threadNumber = -1;

    std::shared_ptr<Level> level = nullptr;
};

OpenMPContext& getOpenMPContext();

void setUpOpenMPContext(const faabric::Message& msg);

void setUpOpenMPContext(int threadNum, std::shared_ptr<Level>& level);
}
