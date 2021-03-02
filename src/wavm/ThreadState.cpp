#include <wavm/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/util/config.h>

using namespace faabric::util;

using namespace WAVM;

namespace wasm {

OpenMPContext& getOpenMPContext()
{
    thread_local static OpenMPContext ctx;
    return ctx;
}

Level::Level(const std::shared_ptr<Level>& parent, int numThreadsIn)
  : depth(parent->depth + 1)
  , effectiveDepth(numThreadsIn > 1 ? parent->effectiveDepth + 1
                                    : parent->effectiveDepth)
  , maxActiveLevels(parent->maxActiveLevels)
  , numThreads(numThreadsIn)
{
    if (numThreads > 1) {
        barrier = std::make_unique<faabric::util::Barrier>(numThreads);
    }
}

Level::Level(int depthIn,
             int effectiveDepthIn,
             int maxActiveLevelsIn,
             int numThreadsIn)
  : depth(depthIn + 1)
  , effectiveDepth(numThreadsIn > 1 ? effectiveDepthIn + 1 : effectiveDepthIn)
  , maxActiveLevels(maxActiveLevelsIn)
  , numThreads(numThreadsIn)
{
    if (numThreads > 1) {
        barrier = std::make_unique<faabric::util::Barrier>(numThreads);
    }
}

int Level::getMaxThreadsAtNextLevel() const
{
    // Limit to one thread if we have exceeded maximum parallelism depth
    if (effectiveDepth >= maxActiveLevels) {
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
