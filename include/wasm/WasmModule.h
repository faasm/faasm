#pragma once

#include <wavm/openmp/Level.h>
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
    class WasmModule {
    public:
        // ----- Module lifecycle -----
        virtual void bindToFunction(const message::Message &msg) = 0;

        virtual void bindToFunctionNoZygote(const message::Message &msg) = 0;

        virtual bool execute(message::Message &msg) = 0;

        virtual const bool isBound() = 0;

        std::string getBoundUser();

        std::string getBoundFunction();

        // ----- Legacy argc/ argv -----
        uint32_t getArgc();

        uint32_t getArgvBufferSize();

        virtual void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) = 0;

        // ----- Environment variables
        virtual void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) = 0;

        WasmEnvironment &getWasmEnvironment();

        // ----- Filesystem -----
        storage::FileSystem &getFileSystem();

        // ----- Stdout capture -----
        ssize_t captureStdout(const struct iovec *iovecs, int iovecCount);

        ssize_t captureStdout(const void *buffer);

        std::string getCapturedStdout();

        void clearCapturedStdout();

        // ----- CoW memory -----
        virtual void writeMemoryToFd(int fd) = 0;

        virtual void mapMemoryFromFd() = 0;

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

        virtual void doSnapshot(std::ostream &outStream) = 0;

        virtual void doRestore(std::istream &inStream) = 0;

        void prepareArgcArgv(const message::Message &msg);

        void prepareOpenMPContext(const message::Message &msg);
};

    // ----- Global functions -----
    message::Message *getExecutingCall();

    void setExecutingCall(message::Message *other);

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