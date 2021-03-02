#include <wavm/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/util/config.h>

using namespace faabric::util;

using namespace WAVM;

namespace wasm {

thread_local int thisThreadNumber = 0;
thread_local std::shared_ptr<Level> thisLevel = nullptr;
thread_local int wantedNumThreads = 1;
thread_local int pushedNumThreads = -1;

OpenMPContext& getOpenMPContext() {
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
    if (pushedNumThreads > 0) {
        return pushedNumThreads;
    }

    // REturn wanted number if set
    if (wantedNumThreads > 0) {
        return wantedNumThreads;
    }

    int defaultNumThreads = (int)faabric::util::getSystemConfig().maxNodes;
    return defaultNumThreads;
}

ReduceTypes SingleHostLevel::reductionMethod()
{
    // There exists many reduction methods, simply implement everything as a
    // critical block unless we know we can avoid synchronisation for now
    if (numThreads == 1) {
        return ReduceTypes::emptyBlock;
    }

    return ReduceTypes::criticalBlock;
}

SingleHostLevel::SingleHostLevel(const std::shared_ptr<Level>& parent,
                                 int numThreads)
  : Level(std::move(parent), numThreads)
{}

MultiHostSumLevel::MultiHostSumLevel(int Depth,
                                     int effectiveDepth,
                                     int maxActiveLevels,
                                     int numThreads)
  : Level(Depth, effectiveDepth, maxActiveLevels, numThreads)
{}

ReduceTypes MultiHostSumLevel::reductionMethod()
{
    return ReduceTypes::multiHostSum;
}

void setUpOpenMPContext(const int threadId, std::shared_ptr<Level> &level) {
    OpenMPContext& ctx = getOpenMPContext();
    ctx.threadNumber = threadId;
    ctx.level = level;
}

void setUpOpenMPContext(const faabric::Message& msg) {
    OpenMPContext& ctx = getOpenMPContext();

    if (msg.ompdepth() > 0) {
        ctx.level = std::static_pointer_cast<Level>(
          std::make_shared<MultiHostSumLevel>(msg.ompdepth(),
                                                      msg.ompeffdepth(),
                                                      msg.ompmal(),
                                                      msg.ompnumthreads()));
    } else {
        ctx.level = std::static_pointer_cast<Level>(
          std::make_shared<SingleHostLevel>());
    }

    ctx.threadNumber = msg.ompthreadnum();
}

}
