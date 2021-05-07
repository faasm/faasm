#include "wasm/WasmModule.h"

#include <conf/FaasmConfig.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/gids.h>
#include <faabric/util/locks.h>
#include <faabric/util/memory.h>
#include <faabric/util/timing.h>
#include <threads/ThreadState.h>

#include <boost/filesystem.hpp>
#include <sstream>
#include <sys/mman.h>
#include <sys/uio.h>

namespace wasm {
// Using TLS here to isolate between executing functions
static thread_local faabric::Message* executingCall = nullptr;
static thread_local wasm::WasmModule* executingModule = nullptr;

bool isWasmPageAligned(int32_t offset)
{
    if (offset & (WASM_BYTES_PER_PAGE - 1)) {
        return false;
    } else {
        return true;
    }
}

faabric::Message* getExecutingCall()
{
    assert(executingCall != nullptr);
    return executingCall;
}

void setExecutingCall(faabric::Message* other)
{
    executingCall = other;
}

wasm::WasmModule* getExecutingModule()
{
    assert(executingModule != nullptr);
    return executingModule;
}

void setExecutingModule(wasm::WasmModule* module)
{
    executingModule = module;
}

size_t getNumberOfWasmPagesForBytes(uint32_t nBytes)
{
    // Round up to nearest page
    size_t pageCount =
      (size_t(nBytes) + WASM_BYTES_PER_PAGE - 1) / WASM_BYTES_PER_PAGE;

    return pageCount;
}

uint32_t roundUpToWasmPageAligned(uint32_t nBytes)
{
    size_t nPages = getNumberOfWasmPagesForBytes(nBytes);
    return (uint32_t)(nPages * WASM_BYTES_PER_PAGE);
}

size_t getPagesForGuardRegion()
{
    size_t regionSize = GUARD_REGION_SIZE;
    size_t nWasmPages = getNumberOfWasmPagesForBytes(regionSize);
    return nWasmPages;
}

// NOTE - the thread pool size here must match up with that for the executor in
// Faabric
WasmModule::WasmModule()
  : threadPoolSize(faabric::util::getUsableCores())
{}

WasmModule::~WasmModule()
{
    // Does nothing
}

void WasmModule::flush() {}

storage::FileSystem& WasmModule::getFileSystem()
{
    return filesystem;
}

wasm::WasmEnvironment& WasmModule::getWasmEnvironment()
{
    return wasmEnvironment;
}

std::string WasmModule::snapshot(bool locallyRestorable)
{
    PROF_START(wasmSnapshot)

    // Create snapshot key
    uint32_t gid = faabric::util::generateGid();
    std::string snapKey =
      this->boundUser + "_" + this->boundFunction + "_" + std::to_string(gid);

    // Note - we only want to take the snapshot to the current brk, not the top
    // of the allocated memory
    faabric::util::SnapshotData data;
    data.data = getMemoryBase();
    data.size = getCurrentBrk();

    faabric::snapshot::SnapshotRegistry& reg =
      faabric::snapshot::getSnapshotRegistry();
    reg.takeSnapshot(snapKey, data, locallyRestorable);

    PROF_END(wasmSnapshot)

    return snapKey;
}

void WasmModule::restore(const std::string& snapshotKey)
{
    PROF_START(wasmSnapshotRestore)

    faabric::snapshot::SnapshotRegistry& reg =
      faabric::snapshot::getSnapshotRegistry();

    // Expand memory if necessary
    faabric::util::SnapshotData data = reg.getSnapshot(snapshotKey);
    uint32_t memSize = getCurrentBrk();

    const auto& logger = faabric::util::getLogger();
    if (data.size > memSize) {
        logger->debug("Growing memory to fit snapshot");
        size_t bytesRequired = data.size - memSize;
        this->growMemory(bytesRequired);
    } else {
        logger->debug("Shrinking memory to fit snapshot");
        size_t shrinkBy = memSize - data.size;
        this->shrinkMemory(shrinkBy);
    }

    // Map the snapshot into memory
    uint8_t* memoryBase = getMemoryBase();
    reg.mapSnapshot(snapshotKey, memoryBase);

    PROF_END(wasmSnapshotRestore)
}

std::string WasmModule::getBoundUser()
{
    return boundUser;
}

std::string WasmModule::getBoundFunction()
{
    return boundFunction;
}

int WasmModule::getStdoutFd()
{
    if (stdoutMemFd == 0) {
        stdoutMemFd = memfd_create("stdoutfd", 0);
        faabric::util::getLogger()->debug("Capturing stdout: fd={}",
                                          stdoutMemFd);
    }

    return stdoutMemFd;
}

ssize_t WasmModule::captureStdout(const struct iovec* iovecs, int iovecCount)
{
    int memFd = getStdoutFd();
    ssize_t writtenSize = ::writev(memFd, iovecs, iovecCount);

    if (writtenSize < 0) {
        faabric::util::getLogger()->error("Failed capturing stdout: {}",
                                          strerror(errno));
        throw std::runtime_error(std::string("Failed capturing stdout: ") +
                                 strerror(errno));
    }

    faabric::util::getLogger()->debug("Captured {} bytes of formatted stdout",
                                      writtenSize);
    stdoutSize += writtenSize;
    return writtenSize;
}

ssize_t WasmModule::captureStdout(const void* buffer)
{
    int memFd = getStdoutFd();

    ssize_t writtenSize =
      dprintf(memFd, "%s\n", reinterpret_cast<const char*>(buffer));

    if (writtenSize < 0) {
        faabric::util::getLogger()->error("Failed capturing stdout: {}",
                                          strerror(errno));
        throw std::runtime_error("Failed capturing stdout");
    }

    faabric::util::getLogger()->debug("Captured {} bytes of unformatted stdout",
                                      writtenSize);
    stdoutSize += writtenSize;
    return writtenSize;
}

std::string WasmModule::getCapturedStdout()
{
    if (stdoutSize == 0) {
        return "";
    }

    // Go back to start
    int memFd = getStdoutFd();
    lseek(memFd, 0, SEEK_SET);

    // Read in and return
    char* buf = new char[stdoutSize];
    read(memFd, buf, stdoutSize);
    std::string stdoutString(buf, stdoutSize);
    faabric::util::getLogger()->debug(
      "Read stdout length {}:\n{}", stdoutSize, stdoutString);

    return stdoutString;
}

void WasmModule::clearCapturedStdout()
{
    close(stdoutMemFd);
    stdoutMemFd = 0;
    stdoutSize = 0;
}

uint32_t WasmModule::getArgc()
{
    return argc;
}

uint32_t WasmModule::getArgvBufferSize()
{
    return argvBufferSize;
}

void WasmModule::prepareArgcArgv(const faabric::Message& msg)
{
    // Here we set up the arguments to main(), i.e. argc and argv
    // We allow passing of arbitrary commandline arguments via the invocation
    // message. These are passed as a string with a space separating each
    // argument.
    argv = faabric::util::getArgvForMessage(msg);
    argc = argv.size();

    // Work out the size of the buffer to hold the strings (allowing
    // for null terminators)
    argvBufferSize = 0;
    for (const auto& thisArg : argv) {
        argvBufferSize += thisArg.size() + 1;
    }
}

/**
 * Maps the given state into the module's memory.
 *
 * If we are dealing with a chunk of a larger state value, the host memory
 * will be reserved for the full value, but only the necessary wasm pages
 * will be created. Loading many chunks of the same value leads to
 * fragmentation, but usually only one or two chunks are loaded per module.
 *
 * To perform the mapping we need to ensure allocated memory is page-aligned.
 */
uint32_t WasmModule::mapSharedStateMemory(
  const std::shared_ptr<faabric::state::StateKeyValue>& kv,
  long offset,
  uint32_t length)
{
    // See if we already have this segment mapped into memory
    std::string segmentKey = kv->user + "_" + kv->key + "__" +
                             std::to_string(offset) + "__" +
                             std::to_string(length);
    if (sharedMemWasmPtrs.count(segmentKey) == 0) {
        // Lock and double check
        faabric::util::UniqueLock lock(moduleStateMutex);
        if (sharedMemWasmPtrs.count(segmentKey) == 0) {
            // Page-align the chunk
            faabric::util::AlignedChunk chunk =
              faabric::util::getPageAlignedChunk(offset, length);

            // Create the wasm memory region and work out the offset to the
            // start of the desired chunk in this region (this will be zero if
            // the offset is already zero, or if the offset is page-aligned
            // already).
            // We need to round the allocation up to a wasm page boundary
            uint32_t allocSize = roundUpToWasmPageAligned(chunk.nBytesLength);
            uint32_t wasmBasePtr = this->growMemory(allocSize);
            uint32_t wasmOffsetPtr = wasmBasePtr + chunk.offsetRemainder;

            // Map the shared memory
            uint8_t* wasmMemoryRegionPtr = wasmPointerToNative(wasmBasePtr);
            kv->mapSharedMemory(static_cast<void*>(wasmMemoryRegionPtr),
                                chunk.nPagesOffset,
                                chunk.nPagesLength);

            // Cache the wasm pointer
            sharedMemWasmPtrs[segmentKey] = wasmOffsetPtr;
        }
    }

    // Return the wasm pointer
    return sharedMemWasmPtrs[segmentKey];
}

uint32_t WasmModule::getCurrentBrk()
{
    faabric::util::SharedLock lock(moduleMemoryMutex);
    return currentBrk;
}

int32_t WasmModule::executeTask(
  int threadPoolIdx,
  int msgIdx,
  std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    setExecutingModule(this);
    faabric::Message& msg = req->mutable_messages()->at(msgIdx);

    assert(boundUser == msg.user());
    assert(boundFunction == msg.function());

    setExecutingCall(&msg);

    uint32_t stackTop = threadStacks.at(threadPoolIdx);

    // Perform the appropriate type of execution
    int returnValue;
    if (req->type() == faabric::BatchExecuteRequest::THREADS) {
        // Pthreads or openmp
        if (req->subtype() == ThreadRequestType::PTHREAD) {
            returnValue = executePthread(threadPoolIdx, stackTop, msg);
        } else if (req->subtype() == ThreadRequestType::OPENMP) {
            threads::setCurrentOpenMPLevel(req);
            returnValue = executeOMPThread(threadPoolIdx, stackTop, msg);
        } else {
            throw std::runtime_error("Unrecognised thread type");
        }
    } else {
        // Vanilla function
        returnValue = executeFunction(msg);
    }

    // Add captured stdout if necessary
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    if (conf.captureStdout == "on") {
        std::string moduleStdout = getCapturedStdout();
        if (!moduleStdout.empty()) {
            std::string newOutput = moduleStdout + "\n" + msg.outputdata();
            msg.set_outputdata(newOutput);

            clearCapturedStdout();
        }
    }

    return returnValue;
}

uint32_t WasmModule::createMemoryGuardRegion(uint32_t wasmOffset)
{
    auto logger = faabric::util::getLogger();

    uint32_t regionSize = GUARD_REGION_SIZE;
    uint8_t* nativePtr = wasmPointerToNative(wasmOffset);

    // NOTE: we want to protect these regions from _writes_, but we don't
    // want to stop them being read, otherwise snapshotting will fail.
    // Therefore we make them read-only
    int res = mprotect(nativePtr, regionSize, PROT_READ);
    if (res != 0) {
        logger->error("Failed to create memory guard: {}",
                      std::strerror(errno));
        throw std::runtime_error("Failed to create memory guard");
    }

    logger->trace(
      "Created guard region {}-{}", wasmOffset, wasmOffset + regionSize);

    return wasmOffset + regionSize;
}

void WasmModule::createThreadStacks()
{
    auto logger = faabric::util::getLogger();

    logger->debug("Creating {} thread stacks", threadPoolSize);

    for (int i = 0; i < threadPoolSize; i++) {
        // Allocate thread and guard pages
        uint32_t memSize = THREAD_STACK_SIZE + (2 * GUARD_REGION_SIZE);
        uint32_t memBase = growMemory(memSize);

        // Note that wasm stacks grow downwards, so we have to store the stack
        // top, which is the offset one below the guard region above the stack
        uint32_t stackTop = memBase + GUARD_REGION_SIZE + THREAD_STACK_SIZE - 1;
        threadStacks.push_back(stackTop);

        // Add guard regions
        createMemoryGuardRegion(memBase);
        createMemoryGuardRegion(stackTop + 1);
    }
}

threads::MutexManager& WasmModule::getMutexes()
{
    return mutexes;
}

// ------------------------------------------
// Functions to be implemented by subclasses
// ------------------------------------------

void WasmModule::reset(const faabric::Message& msg)
{
    faabric::util::getLogger()->warn("Using default reset of wasm module");
}

void WasmModule::bindToFunction(const faabric::Message& msg)
{
    throw std::runtime_error("bindToFunction not implemented");
}

void WasmModule::bindToFunctionNoZygote(const faabric::Message& msg)
{
    throw std::runtime_error("bindToFunctionNoZygote not implemented");
}

bool WasmModule::isBound()
{
    throw std::runtime_error("isBound not implemented");
}

void WasmModule::writeArgvToMemory(uint32_t wasmArgvPointers,
                                   uint32_t wasmArgvBuffer)
{
    throw std::runtime_error("writeArgvToMemory not implemented");
}

void WasmModule::writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer)
{
    throw std::runtime_error("writeWasmEnvToMemory not implemented");
}

uint32_t WasmModule::growMemory(uint32_t nBytes)
{
    throw std::runtime_error("growMemory not implemented");
}

uint32_t WasmModule::shrinkMemory(uint32_t nBytes)
{
    throw std::runtime_error("shrinkMemory not implemented");
}

uint32_t WasmModule::mmapMemory(uint32_t nBytes)
{
    throw std::runtime_error("mmapMemory not implemented");
}

uint32_t WasmModule::mmapFile(uint32_t fp, uint32_t length)
{
    throw std::runtime_error("mmapFile not implemented");
}

void WasmModule::unmapMemory(uint32_t offset, uint32_t nBytes)
{
    throw std::runtime_error("unmapMemory not implemented");
}

uint8_t* WasmModule::wasmPointerToNative(int32_t wasmPtr)
{
    throw std::runtime_error("wasmPointerToNative not implemented");
}

void WasmModule::printDebugInfo()
{
    throw std::runtime_error("printDebugInfo not implemented");
}

size_t WasmModule::getMemorySizeBytes()
{
    throw std::runtime_error("getMemorySizeBytes not implemented");
}

uint8_t* WasmModule::getMemoryBase()
{
    throw std::runtime_error("getMemoryBase not implemented");
}

int32_t WasmModule::executeFunction(faabric::Message& msg)
{
    throw std::runtime_error("executeFunction not implemented");
}

int32_t WasmModule::executeOMPThread(int threadPoolIdx,
                                     uint32_t stackTop,
                                     faabric::Message& msg)
{
    throw std::runtime_error("executeOMPThread not implemented ");
}

int32_t WasmModule::executePthread(int32_t threadPoolIdx,
                                   uint32_t stackTop,
                                   faabric::Message& msg)
{
    throw std::runtime_error("executePthread not implemented");
}
}
