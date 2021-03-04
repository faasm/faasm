#include <wavm/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/util/config.h>

using namespace faabric::util;

using namespace WAVM;

namespace wasm {

OpenMPContext& getOpenMPContext()
{
    static thread_local OpenMPContext ctx;
    return ctx;
}

Level::Level(const std::shared_ptr<Level>& parent, int numThreadsIn)
  : depth(parent->depth + 1)
  , activeLevels(numThreadsIn > 1 ? parent->activeLevels + 1
                                  : parent->activeLevels)
  , maxActiveLevels(parent->maxActiveLevels)
  , numThreads(numThreadsIn)
  , barrier(numThreadsIn)
{}

Level::Level(int depthIn,
             int activeLevelsIn,
             int maxActiveLevelsIn,
             int numThreadsIn)
  : depth(depthIn + 1)
  , activeLevels(numThreadsIn > 1 ? activeLevelsIn + 1 : activeLevelsIn)
  , maxActiveLevels(maxActiveLevelsIn)
  , numThreads(numThreadsIn)
  , barrier(numThreadsIn)
{}

int Level::getMaxThreadsAtNextLevel() const
{
    // Limit to one thread if we have exceeded maximum parallelism depth
    if ((maxActiveLevels > 0) && (activeLevels >= maxActiveLevels)) {
        return 1;
    }

    // Return pushed number if set
    if (pushedThreads > 0) {
        return pushedThreads;
    }

    // Return wanted number if set
    if (wantedThreads > 0) {
        return wantedThreads;
    }

    int defaultNumThreads = (int)faabric::util::getSystemConfig().maxNodes;
    return defaultNumThreads;
}

void Level::masterWait(int threadNum)
{
    // All threads must lock when entering this function
    std::unique_lock<std::mutex> lock(nowaitMutex);

    if (threadNum == 0) {
        // Wait until all non-master threads have finished
        while (nowaitCount < numThreads - 1) {
            nowaitCv.wait(lock);
        }

        // Reset, after we've finished
        nowaitCount = 0;
    } else {
        // Notify master that this thread has finished and continue
        nowaitCount++;
        nowaitCv.notify_one();
    }
}

void setUpOpenMPContext(const int threadId, std::shared_ptr<Level>& level)
{
    OpenMPContext& ctx = getOpenMPContext();
    ctx.threadNumber = threadId;
    ctx.level = level;
}

void setUpOpenMPContext(const faabric::Message& msg)
{
    OpenMPContext& ctx = getOpenMPContext();

    ctx.level = std::make_shared<Level>(
      msg.ompdepth(), msg.ompeffdepth(), msg.ompmal(), msg.ompnumthreads());

    ctx.threadNumber = msg.ompthreadnum();
}
}
