#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/gids.h>
#include <faabric/util/latch.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <faabric/util/timing.h>

#include <threads/ThreadState.h>

using namespace faabric::util;

#define FROM_MAP(varName, T, m, ...)                                           \
    {                                                                          \
        if (m.find(id) == m.end()) {                                           \
            faabric::util::UniqueLock lock(sharedMutex);                       \
            if (m.find(id) == m.end()) {                                       \
                m[id] = std::make_shared<T>(__VA_ARGS__);                      \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    std::shared_ptr<T> varName = m[id];

namespace threads {

static thread_local std::shared_ptr<Level> currentLevel = nullptr;

std::mutex sharedMutex;

std::unordered_map<uint32_t, std::shared_ptr<faabric::util::Latch>> latches;

std::unordered_map<uint32_t, std::shared_ptr<std::recursive_mutex>>
  levelMutexes;

std::unordered_map<uint32_t, std::shared_ptr<std::mutex>> nowaitMutexes;
std::unordered_map<uint32_t, std::shared_ptr<std::atomic<int>>> nowaitCounts;
std::unordered_map<uint32_t, std::shared_ptr<std::condition_variable>>
  nowaitCvs;

void clearThreadState()
{
    latches.clear();

    levelMutexes.clear();

    nowaitMutexes.clear();
    nowaitCounts.clear();
    nowaitCvs.clear();
}

void setCurrentOpenMPLevel(const std::shared_ptr<Level>& level)
{
    currentLevel = level;
}

void setCurrentOpenMPLevel(
  const std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    if (req->contextdata().empty()) {
        SPDLOG_ERROR("Empty OpenMP context for {}", req->id());
        throw std::runtime_error("Empty context for OpenMP request");
    }

    std::string funcStr = faabric::util::funcToString(req);
    SPDLOG_TRACE("Deserialising OpenMP level with {} bytes from {}",
                 req->contextdata().size(),
                 funcStr);

    currentLevel = levelFromBatchRequest(req);
    SPDLOG_TRACE("Set OpenMP level depth {} with {} threads for {}",
                 currentLevel->depth,
                 currentLevel->numThreads,
                 funcStr);
}

std::shared_ptr<Level> getCurrentOpenMPLevel()
{
    if (currentLevel == nullptr) {
        int nThreads =
          faabric::scheduler::getScheduler().getThisHostResources().slots();
        SPDLOG_DEBUG("Creating default OpenMP level with {} threads", nThreads);
        currentLevel = std::make_shared<Level>(nThreads);
    }

    return currentLevel;
}

std::shared_ptr<Level> levelFromBatchRequest(
  const std::shared_ptr<faabric::BatchExecuteRequest>& req)
{
    const auto* other =
      reinterpret_cast<const Level*>(req->contextdata().data());

    auto lvl = std::make_shared<Level>(other->numThreads);
    lvl->deserialise(other);

    return lvl;
}

Level::Level(int numThreadsIn)
  : id(faabric::util::generateGid())
  , numThreads(numThreadsIn)
{}

std::vector<uint32_t> Level::getSharedVarOffsets()
{
    return std::vector<uint32_t>(sharedVarOffsets,
                                 sharedVarOffsets + nSharedVarOffsets);
}

void Level::setSharedVarOffsets(uint32_t* ptr, int nVars)
{
    sharedVarOffsets = new uint32_t[nVars];
    nSharedVarOffsets = nVars;

    std::memcpy(sharedVarOffsets, ptr, nVars * sizeof(uint32_t));
}

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
    FROM_MAP(nowaitMutex, std::mutex, nowaitMutexes)
    FROM_MAP(nowaitCount, std::atomic<int>, nowaitCounts)
    FROM_MAP(nowaitCv, std::condition_variable, nowaitCvs)

    // All threads must lock when entering this function
    std::unique_lock<std::mutex> lock(*nowaitMutex);

    if (threadNum == 0) {
        // Wait until all non-master threads have finished
        while (nowaitCount->load() < numThreads - 1) {
            nowaitCv->wait(lock);
        }

        // Reset, after we've finished
        nowaitCount->store(0);
    } else {
        // If this is the last non-master thread, notify
        int countBefore = nowaitCount->fetch_add(1);
        if (countBefore == numThreads - 2) {
            nowaitCv->notify_one();
        }
    }
}

std::vector<uint8_t> Level::serialise()
{
    // Work out the size of this object
    size_t thisSize = sizeof(Level);
    thisSize += nSharedVarOffsets * sizeof(uint32_t);

    uint8_t* bytesPtr = BYTES(this);
    std::vector<uint8_t> bytes(bytesPtr, bytesPtr + thisSize);
    return bytes;
}

void Level::deserialise(const Level* other)
{
    const auto* otherInts = reinterpret_cast<const int32_t*>(other);

    int nParams = 9;
    printf("Level: ");
    for (int i = 0; i < nParams; i++) {
        printf("%i ", otherInts[i]);
    }

    id = other->id;
    depth = other->depth;
    activeLevels = other->activeLevels;
    maxActiveLevels = other->maxActiveLevels;
    numThreads = other->numThreads;
    pushedThreads = other->pushedThreads;
    wantedThreads = other->wantedThreads;

    globalTidOffset = other->globalTidOffset;

    nSharedVarOffsets = other->nSharedVarOffsets;

    uint32_t* otherShared = other->sharedVarOffsets;
    assert(otherShared != nullptr);

    printf("\nShared: [ ");
    for (int i = 0; i < other->nSharedVarOffsets; i++) {
        printf("%i ", otherShared[i]);
    }
    printf("]\n");

    if (nSharedVarOffsets > 0) {
        sharedVarOffsets = new uint32_t[nSharedVarOffsets];
        std::memcpy(sharedVarOffsets,
                    other->sharedVarOffsets,
                    nSharedVarOffsets * sizeof(uint32_t));
    } else {
        sharedVarOffsets = nullptr;
    }
}

void Level::waitOnBarrier()
{
    // Ignore if single threaded
    if (numThreads <= 1) {
        return;
    }

    // Create if necessary
    if (latches.find(id) == latches.end()) {
        faabric::util::UniqueLock lock(sharedMutex);
        if (latches.find(id) == latches.end()) {
            latches[id] = Latch::create(numThreads);
        }
    }

    latches[id]->wait();

    // Remove the used latch from the map
    if (latches.find(id) != latches.end()) {
        faabric::util::UniqueLock lock(sharedMutex);
        if (latches.find(id) != latches.end()) {
            latches.erase(id);
        }
    }
}

void Level::lockCritical()
{
    FROM_MAP(mx, std::recursive_mutex, levelMutexes);
    mx->lock();
}

void Level::unlockCritical()
{
    FROM_MAP(mx, std::recursive_mutex, levelMutexes);
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

    if (localThreadNum < 0) {
        SPDLOG_ERROR("Local thread num negative: {} - {} @ {}",
                     msg->appindex(),
                     globalTidOffset,
                     depth);

        throw std::runtime_error("Error in local thread number calculation");
    }

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
