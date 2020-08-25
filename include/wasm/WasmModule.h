#pragma once

#include "WasmEnvironment.h"

#include <faabric/util/logging.h>
#include <faabric/state/State.h>
#include <proto/faabric.pb.h>

#include <exception>
#include <string>
#include <tuple>
#include <thread>
#include <mutex>

#include <storage/FileSystem.h>

#define ONE_MB_BYTES 1024 * 1024

#define WASM_BYTES_PER_PAGE 65536

// Note: this is *not* controlling the size provisioned by the linker, that is hard-coded in the build.
// This variable is just here for reference and must be updated to match the value in the build.
#define STACK_SIZE 4 * ONE_MB_BYTES

// Properties of dynamic modules - NOTE - these MUST be passed when compiling the modules themselves
// Heap size must be wasm-module-page-aligned. One page is 64kB so 480 pages is 30MB
#define DYNAMIC_MODULE_STACK_SIZE 2 * ONE_MB_BYTES
#define DYNAMIC_MODULE_HEAP_PAGES 480

// Special known function names
// Zygote function (must match faasm.h linked into the functions themselves)
#define ZYGOTE_FUNC_NAME "_faasm_zygote"
#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define ENTRY_FUNC_NAME "_start"


namespace wasm {
    class WasmModule {
    public:
        virtual ~WasmModule();

        // ----- Module lifecycle -----
        virtual void bindToFunction(const faabric::Message &msg);

        virtual void bindToFunctionNoZygote(const faabric::Message &msg);

        virtual bool execute(faabric::Message &msg, bool forceNoop = false);

        virtual const bool isBound();

        std::string getBoundUser();

        std::string getBoundFunction();

        // ----- Legacy argc/ argv -----
        uint32_t getArgc();

        uint32_t getArgvBufferSize();

        virtual void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer);

        // ----- Environment variables
        virtual void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer);

        WasmEnvironment &getWasmEnvironment();

        // ----- Filesystem -----
        storage::FileSystem &getFileSystem();

        // ----- Stdout capture -----
        ssize_t captureStdout(const struct iovec *iovecs, int iovecCount);

        ssize_t captureStdout(const void *buffer);

        std::string getCapturedStdout();

        void clearCapturedStdout();

        // ----- Memory management -----

        virtual uint32_t mmapMemory(uint32_t length);

        virtual uint32_t mmapPages(uint32_t pages);

        virtual uint32_t mmapFile(uint32_t fp, uint32_t length);

        virtual uint32_t mapSharedStateMemory(const std::shared_ptr<faabric::state::StateKeyValue> &kv, long offset, uint32_t length);

        virtual uint8_t* wasmPointerToNative(int32_t wasmPtr);

        // ----- CoW memory -----
        virtual void writeMemoryToFd(int fd);

        virtual void mapMemoryFromFd();

        // ----- Snapshot/ restore -----
        void snapshotToFile(const std::string &filePath);

        std::vector<uint8_t> snapshotToMemory();

        size_t snapshotToState(const std::string &stateKey);

        void restoreFromFile(const std::string &filePath);

        void restoreFromMemory(const std::vector<uint8_t> &data);

        void restoreFromState(const std::string &stateKey, size_t stateSize);

    protected:
        std::string boundUser;

        std::string boundFunction;

        storage::FileSystem filesystem;

        WasmEnvironment wasmEnvironment;

        int stdoutMemFd;
        ssize_t stdoutSize;

        // Argc/argv
        unsigned int argc;
        std::vector<std::string> argv;
        size_t argvBufferSize;

        int getStdoutFd();

        virtual void doSnapshot(std::ostream &outStream);

        virtual void doRestore(std::istream &inStream);

        void prepareArgcArgv(const faabric::Message &msg);

        // Shared memory regions
        std::mutex sharedMemWasmPtrsMx;
        std::unordered_map<std::string, uint32_t> sharedMemWasmPtrs;
    };

    // ----- Global functions -----
    faabric::Message *getExecutingCall();

    void setExecutingCall(faabric::Message *other);

    // Convenience function
    size_t getNumberOfWasmPagesForBytes(uint32_t nBytes);

    /*
     * Exception thrown when wasm module terminates
     */
    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode) : exitCode(exitCode) {

        }

        int exitCode;
    };

}