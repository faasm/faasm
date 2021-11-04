#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/barrier.h>
#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/gids.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <faabric/util/snapshot.h>
#include <faabric/util/timing.h>

#include <threads/ThreadState.h>

using namespace faabric::util;

#define LEVEL_WAIT_TIMEOUT_MS 20000

namespace threads {

static thread_local std::shared_ptr<Level> currentLevel = nullptr;

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
    currentLevel = levelFromBatchRequest(req);
    SPDLOG_TRACE(
      "Deserialised thread-local OpenMP level from {} bytes for {}, {}",
      req->contextdata().size(),
      funcStr,
      currentLevel->toString());
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
    return Level::deserialise(faabric::util::stringToBytes(req->contextdata()));
}

Level::Level(int numThreadsIn)
  : numThreads(numThreadsIn)
{}

std::vector<uint32_t> Level::getSharedVarOffsets()
{
    return std::vector<uint32_t>(sharedVarOffsets,
                                 sharedVarOffsets + nSharedVarOffsets);
}

void Level::setSharedVarOffsets(uint32_t* ptr, int nVars)
{
    // Record the offsets themselves
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

std::vector<uint8_t> Level::serialise()
{
    // Work out the size of this object
    size_t thisSize = sizeof(Level);
    thisSize += nSharedVarOffsets * sizeof(uint32_t);

    std::vector<uint8_t> bytes(thisSize, 0);

    // Copy the level without the shared offsets (relying on the C++ class
    // memory layout placing members at the top)
    uint8_t* bytesPtr = BYTES(this);
    std::memcpy(bytes.data(), bytesPtr, sizeof(Level));

    // Copy the shared offsets
    if (nSharedVarOffsets > 0) {
        std::memcpy(bytes.data() + sizeof(Level),
                    sharedVarOffsets,
                    nSharedVarOffsets * sizeof(uint32_t));
    }

    SPDLOG_TRACE("Serialising to {} bytes, {}", bytes.size(), toString());

    return bytes;
}

std::shared_ptr<Level> Level::deserialise(const std::vector<uint8_t>& bytes)
{
    // Copy the top section of bytes into a new instance (relying on the
    // C++ class memory layout placing members at the top)
    std::shared_ptr<Level> result = std::make_shared<Level>(0);
    std::memcpy(result.get(), bytes.data(), sizeof(Level));

    // Copy in shared offsets if necessary
    if (result->nSharedVarOffsets > 0) {
        result->sharedVarOffsets = new uint32_t[result->nSharedVarOffsets];
        std::memcpy(result->sharedVarOffsets,
                    bytes.data() + sizeof(Level),
                    result->nSharedVarOffsets * sizeof(uint32_t));
    } else {
        result->sharedVarOffsets = nullptr;
    }

    return result;
}

// Note that we need be able to translate between local and global thread
// numbers. The global thread number must be unique in the system, while the
// local thread number must fit with that expected by OpenMP within the team/
// level. We use Faabric messages to hold the global thread number and can
// translate back and forth.
int Level::getLocalThreadNum(const faabric::Message* msg)
{
    if (depth == 0) {
        return msg->appidx();
    }

    int localThreadNum = msg->appidx() - globalTidOffset;

    if (localThreadNum < 0) {
        SPDLOG_ERROR("Local thread num negative: {} - {} @ {}",
                     msg->appidx(),
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

int Level::getGlobalThreadNum(const faabric::Message* msg)
{
    return msg->appidx();
}

std::string Level::toString()
{
    std::stringstream ss;
    ss << "Level: depth=" << depth << " threads=" << numThreads << " shared={ ";
    for (int i = 0; i < nSharedVarOffsets; i++) {
        ss << sharedVarOffsets[i] << " ";
    }
    ss << "}";

    return ss.str();
}
}
