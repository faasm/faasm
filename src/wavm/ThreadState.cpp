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

Level::Level(int numThreadsIn)
  : numThreads(numThreadsIn)
  , barrier(numThreadsIn)
{}

void Level::fromParentLevel(int parentDepth,
                            int parentActiveLevels,
                            int parentMaxActiveLevels)
{
    depth = parentDepth + 1;

    if (numThreads > 1) {
        activeLevels = parentActiveLevels + 1;
    } else {
        activeLevels = parentActiveLevels;
    }

    maxActiveLevels = parentMaxActiveLevels;
}

void Level::fromParentLevel(const std::shared_ptr<Level>& parent)
{
    return fromParentLevel(
      parent->depth, parent->activeLevels, parent->maxActiveLevels);
}

int Level::getMaxThreadsAtNextLevel() const
{
    // Limit to one thread if the next level exceededs max active levels
    if (activeLevels > maxActiveLevels) {
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

void setUpOpenMPContext(const int threadNum, std::shared_ptr<Level>& thisLevel)
{
    OpenMPContext& ctx = getOpenMPContext();
    ctx.threadNumber = threadNum;
    ctx.level = thisLevel;
}

void setUpOpenMPContext(const faabric::Message& msg)
{
    OpenMPContext& ctx = getOpenMPContext();
    ctx.threadNumber = msg.ompthreadnum();

    // Floor max active levels at 1 for a message. Max active levels of zero
    // disables parallel code.
    int maxActiveLevels = std::max(msg.ompmal(), 1);

    auto level = std::make_shared<Level>(msg.ompnumthreads());
    level->fromParentLevel(msg.ompdepth(), msg.ompeffdepth(), maxActiveLevels);

    ctx.level = level;
}
}
