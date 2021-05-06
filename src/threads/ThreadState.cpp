#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/gids.h>
#include <faabric/util/locks.h>
#include <faabric/util/timing.h>

#include <threads/ThreadState.h>

using namespace faabric::util;

namespace threads {

static thread_local std::shared_ptr<Level> currentLevel = nullptr;

std::mutex sharedMutex;

std::unordered_map<uint32_t, std::shared_ptr<faabric::util::Barrier>> barriers;

std::unordered_map<uint32_t, std::shared_ptr<std::recursive_mutex>>
  levelMutexes;

std::unordered_map<uint32_t, std::shared_ptr<std::mutex>> nowaitMutexes;
std::unordered_map<uint32_t, std::shared_ptr<std::atomic<int>>> nowaitCounts;
std::unordered_map<uint32_t, std::shared_ptr<std::condition_variable>>
  nowaitCvs;

void clearOpenMPState()
{
    barriers.clear();

    levelMutexes.clear();

    nowaitCounts.clear();
    nowaitCvs.clear();
}

std::shared_ptr<std::recursive_mutex> getLevelMutex()
{
    uint32_t id = currentLevel->id;
    if (levelMutexes.count(id) == 0) {
        faabric::util::UniqueLock lock(sharedMutex);
        if (levelMutexes.count(id) == 0) {
            levelMutexes[id] = std::make_shared<std::recursive_mutex>();
        }
    }
    return levelMutexes[id];
}

std::shared_ptr<std::mutex> getNowaitMutex()
{
    uint32_t id = currentLevel->id;
    if (nowaitMutexes.count(id) == 0) {
        faabric::util::UniqueLock lock(sharedMutex);
        if (nowaitMutexes.count(id) == 0) {
            nowaitMutexes[id] = std::make_shared<std::mutex>();
        }
    }
    return nowaitMutexes[id];
}

std::shared_ptr<std::atomic<int>> getNowaitCount()
{
    uint32_t id = currentLevel->id;
    if (nowaitCounts.count(id) == 0) {
        faabric::util::UniqueLock lock(sharedMutex);
        if (nowaitCounts.count(id) == 0) {
            nowaitCounts[id] = std::make_shared<std::atomic<int>>();
        }
    }
    return nowaitCounts[id];
}

std::shared_ptr<std::condition_variable> getNowaitCv()
{
    uint32_t id = currentLevel->id;
    if (nowaitCvs.count(id) == 0) {
        faabric::util::UniqueLock lock(sharedMutex);
        if (nowaitCvs.count(id) == 0) {
            nowaitCvs[id] = std::make_shared<std::condition_variable>();
        }
    }
    return nowaitCvs[id];
}

std::shared_ptr<faabric::util::Barrier> getBarrier()
{
    uint32_t id = currentLevel->id;
    if (barriers.count(id) == 0) {
        faabric::util::UniqueLock lock(sharedMutex);
        if (barriers.count(id) == 0) {
            barriers[id] = std::make_shared<faabric::util::Barrier>(
              currentLevel->numThreads);
        }
    }
    return barriers[id];
}

size_t sizeOfSerialisedLevel(SerialisedLevel& serialisedLevel)
{
    size_t slSize = sizeof(serialisedLevel);
    slSize += serialisedLevel.nSharedVars * sizeof(uint32_t);

    return slSize;
}

void setCurrentOpenMPLevel(const std::shared_ptr<Level>& level)
{
    currentLevel = level;
}

void setCurrentOpenMPLevel(
  const std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    if (req->contextdata().empty()) {
        throw std::runtime_error("Empty context for OpenMP request");
    }

    const SerialisedLevel* serialisedLevel =
      reinterpret_cast<const SerialisedLevel*>(req->contextdata().data());

    currentLevel = std::make_shared<Level>(serialisedLevel->nThreads);
    currentLevel->deserialise(serialisedLevel);
}

std::shared_ptr<Level> getCurrentOpenMPLevel()
{
    if (currentLevel == nullptr) {
        int nThreads =
          faabric::scheduler::getScheduler().getThisHostResources().slots();
        faabric::util::getLogger()->debug(
          "Creating default OpenMP level with {} threads", nThreads);
        currentLevel = std::make_shared<Level>(nThreads);
    }

    return currentLevel;
}

Level::Level(int numThreadsIn)
  : id(faabric::util::generateGid())
  , numThreads(numThreadsIn)
{}

void Level::fromParentLevel(const std::shared_ptr<Level>& parent)
{
    depth = parent->depth + 1;

    if (numThreads > 1) {
        activeLevels = parent->activeLevels + 1;
    } else {
        activeLevels = parent->activeLevels;
    }

    maxActiveLevels = parent->maxActiveLevels;

    if (parent->depth == 0) {
        globalTidOffset = 0;
    } else {
        globalTidOffset = parent->globalTidOffset + parent->numThreads;
    }
}

int Level::getMaxThreadsAtNextLevel() const
{
    // Limit to one thread if the next level exceededs max active levels
    if (activeLevels >= maxActiveLevels) {
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

    int defaultNumThreads = faabric::util::getUsableCores();
    return defaultNumThreads;
}

void Level::masterWait(int threadNum)
{
    // All threads must lock when entering this function
    std::shared_ptr<std::mutex> nowaitMutex = getNowaitMutex();
    std::shared_ptr<std::atomic<int>> nowaitCount = getNowaitCount();
    std::shared_ptr<std::condition_variable> nowaitCv = getNowaitCv();

    std::unique_lock<std::mutex> lock(*nowaitMutex);

    if (threadNum == 0) {
        // Wait until all non-master threads have finished
        while (*nowaitCount < numThreads - 1) {
            nowaitCv->wait(lock);
        }

        // Reset, after we've finished
        *nowaitCount = 0;
    } else {
        // Notify master that this thread has finished and continue
        *nowaitCount += 1;
        nowaitCv->notify_one();
    }
}

SerialisedLevel Level::serialise()
{
    SerialisedLevel sl;
    sl.id = id;
    sl.depth = depth;
    sl.effectiveDepth = activeLevels;
    sl.maxActiveLevels = maxActiveLevels;
    sl.nThreads = numThreads;
    sl.globalTidOffset = globalTidOffset;

    sl.nSharedVars = sharedVarPtrs.size();
    sl.sharedVars = sharedVarPtrs.data();

    return sl;
}

void Level::deserialise(const SerialisedLevel* level)
{
    id = level->id;
    depth = level->depth;
    activeLevels = level->effectiveDepth;
    maxActiveLevels = level->maxActiveLevels;
    numThreads = level->nThreads;
    globalTidOffset = level->globalTidOffset;

    for (int i = 0; i < level->nSharedVars; i++) {
        sharedVarPtrs.emplace_back(level->sharedVars[i]);
    }
}

void Level::waitOnBarrier()
{
    if (numThreads > 1) {
        std::shared_ptr<faabric::util::Barrier> barrier = getBarrier();
        barrier->wait();
    }
}

void Level::lockCritical()
{
    std::shared_ptr<std::recursive_mutex> mx = getLevelMutex();
    mx->lock();
}

void Level::unlockCritical()
{
    std::shared_ptr<std::recursive_mutex> mx = getLevelMutex();
    mx->unlock();
}

// Note that we need be able to translate between local and global thread
// numbers. The global thread number must be unique in the system, while the
// local thread number must fit with that expected by OpenMP within the team/
// level. We use Faabric messages to hold the global thread number and can
// translated back and forth.
int Level::getLocalThreadNum(faabric::Message* msg)
{
    if (depth == 0) {
        return msg->appindex();
    }

    int localThreadNum = msg->appindex() - globalTidOffset;

    if(localThreadNum < 0) {
        faabric::util::getLogger()->error("Local thread num negative: {} - {} @ {}",
                msg->appindex(), globalTidOffset, depth);
    }

    assert(localThreadNum >= 0);
    return localThreadNum;
}

int Level::getGlobalThreadNum(int localThreadNum)
{
    if (depth == 0) {
        return localThreadNum;
    }

    return localThreadNum + globalTidOffset;
}

int Level::getGlobalThreadNum(faabric::Message* msg)
{
    return msg->appindex();
}
}
