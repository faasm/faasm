#include <conf/FaasmConfig.h>
#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/batch.h>
#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/gids.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/memory.h>
#include <faabric/util/snapshot.h>
#include <faabric/util/testing.h>
#include <faabric/util/timing.h>
#include <threads/ThreadState.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

#include <boost/filesystem.hpp>
#include <sstream>
#include <sys/mman.h>
#include <sys/uio.h>

namespace wasm {

bool isWasmPageAligned(int32_t offset)
{
    if (offset & (WASM_BYTES_PER_PAGE - 1)) {
        return false;
    } else {
        return true;
    }
}

size_t getNumberOfWasmPagesForBytes(size_t nBytes)
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

WasmModule::WasmModule()
  : WasmModule(faabric::util::getUsableCores())
{}

WasmModule::WasmModule(int threadPoolSizeIn)
  : threadPoolSize(threadPoolSizeIn)
  , reg(faabric::snapshot::getSnapshotRegistry())
{}

WasmModule::~WasmModule() {}

void WasmModule::flush() {}

storage::FileSystem& WasmModule::getFileSystem()
{
    return filesystem;
}

wasm::WasmEnvironment& WasmModule::getWasmEnvironment()
{
    return wasmEnvironment;
}

std::shared_ptr<faabric::util::SnapshotData> WasmModule::getSnapshotData()
{
    // Note - we only want to take the snapshot to the current brk, not the top
    // of the allocated memory
    uint8_t* memBase = getMemoryBase();
    size_t currentSize = currentBrk.load(std::memory_order_acquire);
    size_t maxSize = MAX_WASM_MEM;

    // Create the snapshot
    auto snap = std::make_shared<faabric::util::SnapshotData>(
      std::span<const uint8_t>(memBase, currentSize), maxSize);

    return snap;
}

std::span<uint8_t> WasmModule::getMemoryView()
{
    uint8_t* memBase = getMemoryBase();
    size_t currentSize = currentBrk.load(std::memory_order_acquire);
    return std::span<uint8_t>(memBase, currentSize);
}

std::string WasmModule::snapshot(bool locallyRestorable)
{
    uint32_t gid = faabric::util::generateGid();
    std::string snapKey =
      this->boundUser + "_" + this->boundFunction + "_" + std::to_string(gid);

    PROF_START(wasmSnapshot)
    std::shared_ptr<faabric::util::SnapshotData> data = getSnapshotData();

    faabric::snapshot::SnapshotRegistry& reg =
      faabric::snapshot::getSnapshotRegistry();
    reg.registerSnapshot(snapKey, data);

    PROF_END(wasmSnapshot)

    return snapKey;
}

void WasmModule::setMemorySize(size_t nBytes)
{
    uint32_t memSize = getCurrentBrk();

    if (nBytes > memSize) {
        size_t bytesRequired = nBytes - memSize;
        SPDLOG_DEBUG("Growing memory by {} bytes to set memory size",
                     bytesRequired);
        this->growMemory(bytesRequired);
    } else if (nBytes < memSize) {
        size_t shrinkBy = memSize - nBytes;
        SPDLOG_DEBUG("Shrinking memory by {} bytes to set memory size",
                     shrinkBy);
        this->shrinkMemory(shrinkBy);
    } else {
        SPDLOG_DEBUG("Memory already correct size for snapshot ({})", memSize);
    }
}

void WasmModule::restore(const std::string& snapshotKey)
{
    if (!isBound()) {
        SPDLOG_ERROR("Must bind wasm module before restoring snapshot {}",
                     snapshotKey);
        throw std::runtime_error("Cannot restore unbound wasm module");
    }

    faabric::snapshot::SnapshotRegistry& reg =
      faabric::snapshot::getSnapshotRegistry();

    // Expand memory if necessary
    auto data = reg.getSnapshot(snapshotKey);
    setMemorySize(data->getSize());

    // Map the snapshot into memory
    uint8_t* memoryBase = getMemoryBase();
    data->mapToMemory({ memoryBase, data->getSize() });
}

void WasmModule::ignoreThreadStacksInSnapshot(const std::string& snapKey)
{
    std::shared_ptr<faabric::util::SnapshotData> snap =
      faabric::snapshot::getSnapshotRegistry().getSnapshot(snapKey);

    // Stacks grow downwards and snapshot diffs are inclusive, so we need to
    // start the diff on the byte at the bottom of the stacks region
    uint32_t threadStackRegionStart =
      threadStacks.at(0) - (THREAD_STACK_SIZE - 1) - GUARD_REGION_SIZE;
    uint32_t threadStackRegionSize =
      threadPoolSize * (THREAD_STACK_SIZE + (2 * GUARD_REGION_SIZE));

    SPDLOG_TRACE("Ignoring snapshot diffs for {} for thread stacks: {}-{}",
                 snapKey,
                 threadStackRegionStart,
                 threadStackRegionStart + threadStackRegionSize);

    // Note - the merge regions for a snapshot are keyed on the offset, so
    // we will just overwrite the same region if another module has already
    // set it
    snap->addMergeRegion(threadStackRegionStart,
                         threadStackRegionSize,
                         faabric::util::SnapshotDataType::Raw,
                         faabric::util::SnapshotMergeOperation::Ignore);
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
        SPDLOG_DEBUG("Capturing stdout: fd={}", stdoutMemFd);
    }

    return stdoutMemFd;
}

ssize_t WasmModule::captureStdout(const struct ::iovec* iovecs, int iovecCount)
{
    int memFd = getStdoutFd();
    ssize_t writtenSize = ::writev(memFd, iovecs, iovecCount);

    if (writtenSize < 0) {
        SPDLOG_ERROR("Failed capturing stdout: {}", strerror(errno));
        throw std::runtime_error(std::string("Failed capturing stdout: ") +
                                 strerror(errno));
    }

    SPDLOG_DEBUG("Captured {} bytes of formatted stdout", writtenSize);
    stdoutSize += writtenSize;
    return writtenSize;
}

ssize_t WasmModule::captureStdout(const void* buffer)
{
    int memFd = getStdoutFd();

    ssize_t writtenSize =
      dprintf(memFd, "%s\n", reinterpret_cast<const char*>(buffer));

    if (writtenSize < 0) {
        SPDLOG_ERROR("Failed capturing stdout: {}", strerror(errno));
        throw std::runtime_error("Failed capturing stdout");
    }

    SPDLOG_DEBUG("Captured {} bytes of unformatted stdout", writtenSize);
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
    std::string stdoutString(stdoutSize, '\0');
    read(memFd, stdoutString.data(), stdoutSize);
    SPDLOG_DEBUG("Read stdout length {}:\n{}", stdoutSize, stdoutString);

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

void WasmModule::bindToFunction(faabric::Message& msg, bool cache)
{
    if (_isBound) {
        throw std::runtime_error("Cannot bind a module twice");
    }

    _isBound = true;
    boundUser = msg.user();
    boundFunction = msg.function();

    // Call into subclass hook, setting the context beforehand
    WasmExecutionContext ctx(this);
    doBindToFunction(msg, cache);
}

void WasmModule::prepareArgcArgv(const faabric::Message& msg)
{
    // Here we set up the arguments to main(), i.e. argc and argv
    // We allow passing of arbitrary commandline arguments via the
    // invocation message. These are passed as a string with a space
    // separating each argument.
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
 * To perform the mapping we need to ensure allocated memory is
 * page-aligned.
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
        faabric::util::FullLock lock(sharedMemWasmPtrsMutex);

        if (sharedMemWasmPtrs.count(segmentKey) == 0) {
            // Page-align the chunk
            faabric::util::AlignedChunk chunk =
              faabric::util::getPageAlignedChunk(offset, length);

            // Create the wasm memory region and work out the offset to the
            // start of the desired chunk in this region (this will be zero
            // if the offset is already zero, or if the offset is
            // page-aligned already). We need to round the allocation up to
            // a wasm page boundary
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
    {
        faabric::util::SharedLock lock(sharedMemWasmPtrsMutex);
        return sharedMemWasmPtrs[segmentKey];
    }
}

uint32_t WasmModule::getCurrentBrk()
{
    return currentBrk.load(std::memory_order_acquire);
}

int32_t WasmModule::executeTask(
  int threadPoolIdx,
  int msgIdx,
  std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    faabric::Message& msg = req->mutable_messages()->at(msgIdx);
    std::string funcStr = faabric::util::funcToString(msg, true);

    if (!isBound()) {
        throw std::runtime_error(
          "WasmModule must be bound before executing anything");
    }

    assert(boundUser == msg.user());
    assert(boundFunction == msg.function());

    // Set up context for this task
    WasmExecutionContext ctx(this);

    // Modules must have provisioned their own thread stacks
    assert(!threadStacks.empty());
    uint32_t stackTop = threadStacks.at(threadPoolIdx);

    // Ignore stacks and guard pages in snapshot if present
    if (!msg.snapshotkey().empty()) {
        ignoreThreadStacksInSnapshot(msg.snapshotkey());
    }

    // Perform the appropriate type of execution
    int returnValue;
    if (req->type() == faabric::BatchExecuteRequest::THREADS) {
        switch (req->subtype()) {
            case ThreadRequestType::PTHREAD: {
                SPDLOG_TRACE("Executing {} as pthread", funcStr);
                returnValue = executePthread(threadPoolIdx, stackTop, msg);
                break;
            }
            case ThreadRequestType::OPENMP: {
                SPDLOG_TRACE("Executing {} as OpenMP (group {}, size {})",
                             funcStr,
                             msg.groupid(),
                             msg.groupsize());

                // Set up the level
                threads::setCurrentOpenMPLevel(req);
                returnValue = executeOMPThread(threadPoolIdx, stackTop, msg);
                break;
            }
            default: {
                SPDLOG_ERROR("{} has unrecognised thread subtype {}",
                             funcStr,
                             req->subtype());
                throw std::runtime_error("Unrecognised thread subtype");
            }
        }
    } else {
        // Vanilla function
        SPDLOG_TRACE("Executing {} as standard function", funcStr);
        returnValue = executeFunction(msg);
    }

    if (returnValue != 0) {
        msg.set_outputdata(
          fmt::format("Call failed (return value={})", returnValue));
    }

    // Add captured stdout if necessary
    conf::FaasmConfig& conf = conf::getFaasmConfig();
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

    uint32_t regionSize = GUARD_REGION_SIZE;
    uint8_t* nativePtr = wasmPointerToNative(wasmOffset);

    // NOTE: we want to protect these regions from _writes_, but we don't
    // want to stop them being read, otherwise snapshotting will fail.
    // Therefore we make them read-only
    int res = mprotect(nativePtr, regionSize, PROT_READ);
    if (res != 0) {
        SPDLOG_ERROR("Failed to create memory guard: {}", std::strerror(errno));
        throw std::runtime_error("Failed to create memory guard");
    }

    SPDLOG_TRACE(
      "Created guard region {}-{}", wasmOffset, wasmOffset + regionSize);

    return wasmOffset + regionSize;
}

void WasmModule::addMergeRegionForNextThreads(
  uint32_t wasmPtr,
  size_t regionSize,
  faabric::util::SnapshotDataType dataType,
  faabric::util::SnapshotMergeOperation mergeOp)
{
    mergeRegions.emplace_back(wasmPtr, regionSize, dataType, mergeOp);
}

std::vector<faabric::util::SnapshotMergeRegion> WasmModule::getMergeRegions()
{
    return mergeRegions;
}

void WasmModule::clearMergeRegions()
{
    mergeRegions.clear();
}

void WasmModule::queuePthreadCall(threads::PthreadCall call)
{
    // We assume that all pthread calls are queued from the main thread before
    // await is called from the same thread, so this doesn't need to be
    // thread-safe.
    queuedPthreadCalls.emplace_back(call);
}

int WasmModule::awaitPthreadCall(faabric::Message* msg, int pthreadPtr)
{
    // We assume that await is called in a loop from the main thread, after
    // all pthread calls have been queued, so this function doesn't need to be
    // thread safe.
    assert(msg != nullptr);

    // Execute the queued pthread calls
    faabric::scheduler::Executor* executor =
      faabric::scheduler::ExecutorContext::get()->getExecutor();
    if (!queuedPthreadCalls.empty()) {
        int nPthreadCalls = queuedPthreadCalls.size();

        std::string funcStr = faabric::util::funcToString(*msg, true);
        SPDLOG_DEBUG(
          "Executing {} pthread calls for {}", nPthreadCalls, funcStr);

        std::shared_ptr<faabric::BatchExecuteRequest> req =
          faabric::util::batchExecFactory(
            msg->user(), msg->function(), nPthreadCalls);
        faabric::util::updateBatchExecAppId(req, msg->appid());

        req->set_type(faabric::BatchExecuteRequest::THREADS);
        req->set_subtype(wasm::ThreadRequestType::PTHREAD);

        // In the local tests, we always set the single-host flag to avoid
        // having to synchronise snapshots
        if (faabric::util::isTestMode()) {
            req->set_singlehost(true);
        }

        for (int i = 0; i < nPthreadCalls; i++) {
            threads::PthreadCall p = queuedPthreadCalls.at(i);
            faabric::Message& m = req->mutable_messages()->at(i);

            // Function pointer and args
            // NOTE - with a pthread interface we only ever pass the
            // function a single pointer argument, hence we use the
            // input data here to hold this argument as a string
            m.set_funcptr(p.entryFunc);
            m.set_inputdata(std::to_string(p.argsPtr));

            // Assign a thread ID and increment. Our pthread IDs start
            // at 1. Set this as part of the group with the other threads.
            m.set_appidx(i + 1);
            m.set_groupidx(i + 1);

            // Record this thread -> call ID
            SPDLOG_TRACE("pthread {} mapped to call {}", p.pthreadPtr, m.id());
            pthreadPtrsToChainedCalls.insert({ p.pthreadPtr, m.id() });
        }

        // Execute the threads and await results
        lastPthreadResults = executor->executeThreads(req, mergeRegions);

        // Empty the queue
        queuedPthreadCalls.clear();
    }

    // Get the result of this call
    unsigned int pthreadMsgId = pthreadPtrsToChainedCalls[pthreadPtr];
    bool found = false;
    int thisResult = 0;
    for (auto [mid, res] : lastPthreadResults) {
        if (pthreadMsgId == mid) {
            thisResult = res;
            found = true;
            break;
        }
    }

    if (!found) {
        SPDLOG_ERROR("Did not find a result for pthread: ptr {}, mid {}",
                     pthreadPtr,
                     pthreadMsgId);
        throw std::runtime_error("Result not found for pthread");
    }

    // Remove the mapping for this pointer
    pthreadPtrsToChainedCalls.erase(pthreadPtr);

    // If we're done, clear the results
    if (pthreadPtrsToChainedCalls.empty()) {
        lastPthreadResults.clear();
    }

    return thisResult;
}

void WasmModule::createThreadStacks()
{
    SPDLOG_DEBUG("Creating {} thread stacks", threadPoolSize);

    for (int i = 0; i < threadPoolSize; i++) {
        // Allocate thread stack and guard pages
        uint32_t memSize = THREAD_STACK_SIZE + (2 * GUARD_REGION_SIZE);
        uint32_t memBase = growMemory(memSize);

        // Note that wasm stacks grow downwards, so we have to store the
        // stack top, which is the offset one below the guard region above
        // the stack Subtract 16 to make sure the stack is 16-aligned as
        // required by the C ABI
        uint32_t stackTop =
          memBase + GUARD_REGION_SIZE + THREAD_STACK_SIZE - 16;
        threadStacks.push_back(stackTop);

        // Add guard regions
        createMemoryGuardRegion(memBase);
        createMemoryGuardRegion(stackTop + 16);
    }
}

std::vector<uint32_t> WasmModule::getThreadStacks()
{
    return threadStacks;
}

std::shared_ptr<std::mutex> WasmModule::getPthreadMutex(uint32_t id)
{
    faabric::util::SharedLock lock(pthreadLocksMx);
    auto it = pthreadLocks.find(id);
    if (it != pthreadLocks.end()) {
        return it->second;
    }

    SPDLOG_ERROR("Trying to get non-existent pthread lock {}", id);
    throw std::runtime_error("Non-existent pthread lock");
}

std::shared_ptr<std::mutex> WasmModule::getOrCreatePthreadMutex(uint32_t id)
{
    std::shared_ptr<std::mutex> mx = nullptr;
    {
        faabric::util::SharedLock lock(pthreadLocksMx);
        auto it = pthreadLocks.find(id);
        if (it != pthreadLocks.end()) {
            return it->second;
        }
    }

    faabric::util::FullLock lock(pthreadLocksMx);
    auto it = pthreadLocks.find(id);
    if (it != pthreadLocks.end()) {
        return it->second;
    }

    mx = std::make_shared<std::mutex>();
    pthreadLocks[id] = mx;

    return mx;
}

bool WasmModule::isBound()
{
    return _isBound;
}

// ------------------------------------------
// Functions to be implemented by subclasses
// ------------------------------------------

void WasmModule::reset(faabric::Message& msg, const std::string& snapshotKey)
{
    SPDLOG_WARN("Using default reset of wasm module");
}

void WasmModule::doBindToFunction(faabric::Message& msg, bool cache)
{
    throw std::runtime_error("doBindToFunction not implemented");
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

uint32_t WasmModule::growMemory(size_t nBytes)
{
    if (nBytes == 0) {
        return currentBrk.load(std::memory_order_acquire);
    }

    faabric::util::FullLock lock(moduleMutex);

    uint32_t oldBytes = getMemorySizeBytes();
    uint32_t oldBrk = currentBrk.load(std::memory_order_acquire);
    uint32_t newBrk = oldBrk + nBytes;

    if (!isWasmPageAligned(newBrk)) {
        SPDLOG_ERROR("Growing memory by {} is not wasm page aligned"
                     " (current brk: {}, new brk: {})",
                     nBytes,
                     oldBrk,
                     newBrk);
        throw std::runtime_error("Non-wasm-page-aligned memory growth");
    }

    size_t newBytes = oldBytes + nBytes;
    uint32_t oldPages = getNumberOfWasmPagesForBytes(oldBytes);
    uint32_t newPages = getNumberOfWasmPagesForBytes(newBytes);
    size_t maxPages = getMaxMemoryPages();

    if (newBytes > UINT32_MAX || newPages > maxPages) {
        SPDLOG_ERROR("Growing memory would exceed max of {} pages (current {}, "
                     "requested {})",
                     maxPages,
                     oldPages,
                     newPages);
        throw std::runtime_error("Memory growth exceeding max");
    }

    // If we can reclaim old memory, just bump the break
    if (newBrk <= oldBytes) {
        SPDLOG_TRACE(
          "MEM - Growing memory using already provisioned {} + {} <= {}",
          oldBrk,
          nBytes,
          oldBytes);

        currentBrk.store(newBrk, std::memory_order_release);

        // Make sure permissions on memory are open
        size_t newTop = faabric::util::getRequiredHostPages(currentBrk);
        size_t newStart = faabric::util::getRequiredHostPagesRoundDown(oldBrk);
        newTop *= faabric::util::HOST_PAGE_SIZE;
        newStart *= faabric::util::HOST_PAGE_SIZE;
        SPDLOG_TRACE("Reclaiming memory {}-{}", newStart, newTop);

        uint8_t* memBase = getMemoryBase();
        faabric::util::claimVirtualMemory(
          { memBase + newStart, memBase + newTop });

        return oldBrk;
    }

    uint32_t pageChange = newPages - oldPages;
    bool success = doGrowMemory(pageChange);
    if (!success) {
        throw std::runtime_error("Failed to grow memory");
    }

    SPDLOG_TRACE("Growing memory from {} to {} pages (max {})",
                 oldPages,
                 newPages,
                 maxPages);

    size_t newMemorySize = getMemorySizeBytes();
    currentBrk.store(newMemorySize, std::memory_order_release);

    if (newMemorySize != newBytes) {
        SPDLOG_ERROR(
          "Expected new brk ({}) to be old memory plus new bytes ({})",
          newMemorySize,
          newBytes);
        throw std::runtime_error("Memory growth discrepancy");
    }

    return oldBrk;
}

bool WasmModule::doGrowMemory(uint32_t pageChange)
{
    throw std::runtime_error("doGrowMemory not implemented");
}

uint32_t WasmModule::shrinkMemory(size_t nBytes)
{
    if (!isWasmPageAligned(nBytes)) {
        SPDLOG_ERROR("Shrink size not page aligned {}", nBytes);
        throw std::runtime_error("New break not page aligned");
    }

    faabric::util::FullLock lock(moduleMutex);

    uint32_t oldBrk = currentBrk.load(std::memory_order_acquire);

    if (nBytes > oldBrk) {
        SPDLOG_ERROR(
          "Shrinking by more than current brk ({} > {})", nBytes, oldBrk);
        throw std::runtime_error("Shrinking by more than current brk");
    }

    // Note - we don't actually free the memory, we just change the brk
    uint32_t newBrk = oldBrk - nBytes;

    SPDLOG_TRACE("MEM - shrinking memory {} -> {}", oldBrk, newBrk);
    currentBrk.store(newBrk, std::memory_order_release);

    return oldBrk;
}

uint32_t WasmModule::mmapMemory(size_t nBytes)
{
    // The mmap interface allows non page-aligned values, and rounds up
    uint32_t pageAligned = roundUpToWasmPageAligned(nBytes);
    return growMemory(pageAligned);
}

uint32_t WasmModule::mmapFile(uint32_t fp, size_t length)
{
    throw std::runtime_error("mmapFile not implemented");
}

void WasmModule::unmapMemory(uint32_t offset, size_t nBytes)
{
    if (nBytes == 0) {
        return;
    }

    // Munmap expects the offset itself to be page-aligned, but will round up
    // the number of bytes
    if (!isWasmPageAligned(offset)) {
        SPDLOG_ERROR("Non-page aligned munmap address {}", offset);
        throw std::runtime_error("Non-page aligned munmap address");
    }

    uint32_t pageAligned = roundUpToWasmPageAligned(nBytes);
    size_t maxPages = getMaxMemoryPages();
    size_t maxSize = maxPages * WASM_BYTES_PER_PAGE;
    uint32_t unmapTop = offset + pageAligned;

    if (unmapTop > maxSize) {
        SPDLOG_ERROR(
          "Munmapping outside memory max ({} > {})", unmapTop, maxSize);
        throw std::runtime_error("munmapping outside memory max");
    }

    if (unmapTop == currentBrk.load(std::memory_order_acquire)) {
        SPDLOG_TRACE("MEM - munmapping top of memory by {}", pageAligned);
        shrinkMemory(pageAligned);
    } else {
        SPDLOG_WARN("MEM - unable to reclaim unmapped memory {} at {}",
                    pageAligned,
                    offset);
    }
}

void WasmModule::doThrowException(std::exception& e)
{
    throw std::runtime_error("doThrowException not implemented");
}

uint8_t* WasmModule::wasmPointerToNative(uint32_t wasmPtr)
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

size_t WasmModule::getMaxMemoryPages()
{
    throw std::runtime_error("getMaxMemoryPages not implemented");
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
