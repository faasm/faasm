#pragma once

#include "WasmEnvironment.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/state/State.h>
#include <faabric/util/logging.h>
#include <faabric/util/memory.h>
#include <faabric/util/queue.h>
#include <faabric/util/snapshot.h>
#include <threads/MutexManager.h>
#include <threads/ThreadState.h>

#include <exception>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>

#include <storage/FileSystem.h>

#define ONE_MB_BYTES 1024 * 1024

#define WASM_BYTES_PER_PAGE 65536

// Note: this is *not* controlling the size provisioned by the linker, that is
// hard-coded in the build. This variable is just here for reference and must be
// updated to match the value in the build.
#define STACK_SIZE (4 * ONE_MB_BYTES)
#define THREAD_STACK_SIZE (2 * ONE_MB_BYTES)

// Properties of dynamic modules. Heap size must be wasm-module-page-aligned.
// One page is 64kB
#define DYNAMIC_MODULE_STACK_SIZE (2 * ONE_MB_BYTES)
#define DYNAMIC_MODULE_MEMORY_SIZE (66 * WASM_BYTES_PER_PAGE)
#define GUARD_REGION_SIZE (10 * WASM_BYTES_PER_PAGE)

// Special known function names
// Zygote function (must match faasm.h linked into the functions themselves)
#define ZYGOTE_FUNC_NAME "_faasm_zygote"
#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define ENTRY_FUNC_NAME "_start"

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
    virtual void reset(const faabric::Message& msg);

    void bindToFunction(const faabric::Message& msg, bool cache = true);

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
    ssize_t captureStdout(const struct iovec* iovecs, int iovecCount);

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

    virtual uint8_t* wasmPointerToNative(int32_t wasmPtr);

    virtual size_t getMemorySizeBytes();

    // ----- Snapshot/ restore -----
    std::string snapshot(bool locallyRestorable = true);

    void restore(const std::string& snapshotKey);

    // ----- Debugging -----
    virtual void printDebugInfo();

    threads::MutexManager& getMutexes();

  protected:
    uint32_t currentBrk = 0;

    std::string boundUser;
    std::string boundFunction;
    bool _isBound = false;

    storage::FileSystem filesystem;

    WasmEnvironment wasmEnvironment;

    int stdoutMemFd = 0;
    ssize_t stdoutSize = 0;

    int threadPoolSize = 0;
    std::vector<uint32_t> threadStacks;

    threads::MutexManager mutexes;

    std::shared_mutex moduleMemoryMutex;
    std::mutex moduleStateMutex;

    // Argc/argv
    unsigned int argc;
    std::vector<std::string> argv;
    size_t argvBufferSize;

    // Shared memory regions
    std::unordered_map<std::string, uint32_t> sharedMemWasmPtrs;

    int getStdoutFd();

    void prepareArgcArgv(const faabric::Message& msg);

    virtual uint8_t* getMemoryBase();

    // Module-specific binding
    virtual void doBindToFunction(const faabric::Message& msg, bool cache);

    // Threads
    void createThreadStacks();
};

// ----- Global functions -----
void setExecutingCall(faabric::Message* call);

faabric::Message* getExecutingCall();

void setExecutingModule(wasm::WasmModule* module);

wasm::WasmModule* getExecutingModule();

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
