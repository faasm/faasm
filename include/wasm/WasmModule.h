#pragma once

#include "WasmEnvironment.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/state/State.h>
#include <faabric/util/memory.h>
#include <faabric/util/queue.h>
#include <faabric/util/snapshot.h>
#include <threads/ThreadState.h>

#include <atomic>
#include <exception>
#include <mutex>
#include <string>
#include <sys/uio.h>
#include <thread>
#include <tuple>

#include <storage/FileSystem.h>

// Special known function names
// Zygote function (must match faasm.h linked into the functions themselves)
#define ZYGOTE_FUNC_NAME "_faasm_zygote"
#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define ENTRY_FUNC_NAME "_start"

#define MAX_WASM_MEM (1024L * 1024L * 1024L * 4L)

namespace wasm {

// Note - avoid a zero default on the thread request type otherwise it can
// go unset without noticing
enum ThreadRequestType
{
    UNSET = 0,
    PTHREAD = 1,
    OPENMP = 2,
};

bool isWasmPageAligned(int32_t offset);

class WasmModule
{
  public:
    WasmModule();

    explicit WasmModule(int threadPoolSizeIn);

    virtual ~WasmModule();

    // ----- Module lifecycle -----
    virtual void reset(faabric::Message& msg, const std::string& snapshotKey);

    void bindToFunction(faabric::Message& msg, bool cache = true);

    int32_t executeTask(int threadPoolIdx,
                        int msgIdx,
                        std::shared_ptr<faabric::BatchExecuteRequest> req);

    virtual int32_t executeFunction(faabric::Message& msg);

    virtual int32_t executeOMPThread(int threadPoolIdx,
                                     uint32_t stackTop,
                                     faabric::Message& msg);

    virtual int32_t executePthread(int threadPoolIdx,
                                   uint32_t stackTop,
                                   faabric::Message& msg);

    bool isBound();

    std::string getBoundUser();

    std::string getBoundFunction();

    virtual void flush();

    // ----- argc/ argv -----
    uint32_t getArgc();

    uint32_t getArgvBufferSize();

    virtual void writeArgvToMemory(uint32_t wasmArgvPointers,
                                   uint32_t wasmArgvBuffer);

    // ----- Environment variables
    virtual void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer);

    WasmEnvironment& getWasmEnvironment();

    // ----- Filesystem -----
    storage::FileSystem& getFileSystem();

    // ----- Stdout capture -----
    ssize_t captureStdout(const struct ::iovec* iovecs, int iovecCount);

    ssize_t captureStdout(const void* buffer);

    std::string getCapturedStdout();

    void clearCapturedStdout();

    // ----- Memory management -----
    uint32_t getCurrentBrk();

    virtual uint32_t growMemory(uint32_t nBytes);

    virtual uint32_t shrinkMemory(uint32_t nBytes);

    virtual uint32_t mmapMemory(uint32_t nBytes);

    virtual uint32_t mmapFile(uint32_t fp, uint32_t length);

    virtual void unmapMemory(uint32_t offset, uint32_t nBytes);

    uint32_t createMemoryGuardRegion(uint32_t wasmOffset);

    virtual uint32_t mapSharedStateMemory(
      const std::shared_ptr<faabric::state::StateKeyValue>& kv,
      long offset,
      uint32_t length);

    virtual uint8_t* wasmPointerToNative(uint32_t wasmPtr);

    virtual size_t getMemorySizeBytes();

    virtual uint8_t* getMemoryBase();

    // ----- Snapshot/ restore -----
    std::shared_ptr<faabric::util::SnapshotData> getSnapshotData();

    faabric::util::MemoryView getMemoryView();

    std::string getOrCreateAppSnapshot(const faabric::Message& msg,
                                       bool update);

    void deleteAppSnapshot(const faabric::Message& msg);

    std::string snapshot(bool locallyRestorable = true);

    void restore(const std::string& snapshotKey);

    // ----- Threading -----
    void queuePthreadCall(threads::PthreadCall call);

    int awaitPthreadCall(const faabric::Message* msg, int pthreadPtr);

    void setUpPthreadMergeRegions(const faabric::Message& msg,
                                  std::shared_ptr<threads::Level> ompLevel);

    std::vector<uint32_t> getThreadStacks();

    // ----- Debugging -----
    virtual void printDebugInfo();

  protected:
    std::shared_mutex moduleMutex;

    std::atomic<uint32_t> currentBrk = 0;

    std::string boundUser;
    std::string boundFunction;
    bool _isBound = false;

    storage::FileSystem filesystem;

    WasmEnvironment wasmEnvironment;

    int stdoutMemFd = 0;
    ssize_t stdoutSize = 0;

    int threadPoolSize = 0;
    std::vector<uint32_t> threadStacks;

    // Argc/argv
    unsigned int argc;
    std::vector<std::string> argv;
    size_t argvBufferSize;

    // Threads
    std::atomic<bool> isQueuedPthreadCalls = false;
    std::vector<threads::PthreadCall> queuedPthreadCalls;
    std::unordered_map<int32_t, uint32_t> pthreadPtrsToChainedCalls;

    // Shared memory regions
    std::shared_mutex sharedMemWasmPtrsMutex;
    std::unordered_map<std::string, uint32_t> sharedMemWasmPtrs;

    int getStdoutFd();

    void prepareArgcArgv(const faabric::Message& msg);

    // Module-specific binding
    virtual void doBindToFunction(faabric::Message& msg, bool cache);

    // Snapshots
    void snapshotWithKey(const std::string& snapKey);

    // Threads
    void createThreadStacks();
};

// Convenience functions
size_t getNumberOfWasmPagesForBytes(uint32_t nBytes);

uint32_t roundUpToWasmPageAligned(uint32_t nBytes);

size_t getPagesForGuardRegion();

/*
 * Exception thrown when wasm module terminates
 */
class WasmExitException : public std::exception
{
  public:
    explicit WasmExitException(int exitCode)
      : exitCode(exitCode)
    {}

    int exitCode;
};

}
