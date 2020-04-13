#pragma once

#include "WasmEnvironment.h"

#include <util/logging.h>
#include <state/State.h>
#include <proto/faasm.pb.h>

#include <exception>
#include <string>
#include <tuple>
#include <thread>
#include <mutex>

#include <storage/FileSystem.h>

#define ONE_MB_BYTES 1024 * 1024

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
    class WasmModule  {
    public:
        // ----- Module lifecycle -----
        virtual void bindToFunction(const message::Message &msg, bool executeZygote) = 0;

        virtual bool execute(message::Message &msg) = 0;

        virtual const bool isBound() = 0;

        virtual std::string getBoundUser() = 0;

        virtual std::string getBoundFunction() = 0;

        // ----- Memory management -----
        virtual uint32_t mmapMemory(uint32_t length) = 0;

        virtual uint32_t mmapPages(uint32_t pages) = 0;

        virtual uint32_t mmapFile(uint32_t fp, uint32_t length) = 0;

        virtual uint32_t mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length) = 0;

        // ----- Legacy argc/ argv -----
        virtual uint32_t getArgc() = 0;

        virtual uint32_t getArgvBufferSize() = 0;

        virtual void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) = 0;

        // ----- Environment variables
        virtual void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) = 0;

        virtual WasmEnvironment &getWasmEnvironment() = 0;

        // ----- Filesystem -----
        virtual storage::FileSystem &getFileSystem() = 0;

        // ----- Stdout capture -----
        virtual ssize_t captureStdout(const struct iovec *iovecs, int iovecCount) = 0;

        virtual ssize_t captureStdout(const void *buffer) = 0;

        virtual std::string getCapturedStdout() = 0;

        virtual void clearCapturedStdout() = 0;

        // ----- Pre-WASI filesystem stuff -----
        virtual void addFdForThisThread(int fd) = 0;

        virtual void removeFdForThisThread(int fd) = 0;

        virtual void clearFds() = 0;

        virtual void checkThreadOwnsFd(int fd) = 0;

        // ----- Typescript -----
        virtual bool getBoundIsTypescript() = 0;

        // ----- CoW memory -----
        virtual void writeMemoryToFd(int fd) = 0;

        virtual void mapMemoryFromFd() = 0;

        // ----- Snapshot/ restore -----
        virtual void snapshotToFile(const std::string &filePath) = 0;

        virtual std::vector<uint8_t> snapshotToMemory() = 0;

        virtual size_t snapshotToState(const std::string &stateKey) = 0;

        virtual void restoreFromFile(const std::string &filePath) = 0;

        virtual void restoreFromMemory(const std::vector<uint8_t> &data) = 0;

        virtual void restoreFromState(const std::string &stateKey, size_t stateSize) = 0;
    };

    // ----- Global functions -----
    WasmModule *getExecutingModule();

    void setExecutingModule(WasmModule *executingModule);

    message::Message *getExecutingCall();

    void setExecutingCall(message::Message *other);

    // Convenience function
    size_t getNumberOfPagesForBytes(uint32_t nBytes);

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