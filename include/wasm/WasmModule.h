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

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>

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

using namespace WAVM;


namespace wasm {
    WAVM_DECLARE_INTRINSIC_MODULE(env)

    WAVM_DECLARE_INTRINSIC_MODULE(wasi)

    WAVM_DECLARE_INTRINSIC_MODULE(tsenv)

    Uptr getNumberOfPagesForBytes(U32 nBytes);

    struct WasmThreadSpec;

    class WasmModule final : Runtime::Resolver {
    public:
        WasmModule();

        WasmModule(const WasmModule &other);

        WasmModule &operator=(const WasmModule &other);

        ~WasmModule();

        void bindToFunction(const message::Message &msg, bool executeZygote = true);

        bool execute(message::Message &msg);

        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        Runtime::GCPointer<Runtime::Table> defaultTable;

        Runtime::GCPointer<Runtime::Context> executionContext;

        Runtime::GCPointer<Runtime::Compartment> compartment;

        const bool isBound();

        U32 mmapMemory(U32 length);

        U32 mmapPages(U32 pages);

        U32 mmapFile(U32 fp, U32 length);

        U32 mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, U32 length);

        int dynamicLoadModule(const std::string &path, Runtime::Context *context);

        Uptr getDynamicModuleFunction(int handle, const std::string &funcName);

        Uptr addFunctionToTable(Runtime::Object *exportedFunc);

        void executeFunction(
                Runtime::Function *func,
                IR::FunctionType funcType,
                const std::vector<IR::UntaggedValue> &arguments,
                IR::UntaggedValue &result
        );

        Runtime::Function *getFunction(Runtime::Instance *module, const std::string &funcName, bool strict);

        Runtime::Function *getFunctionFromPtr(int funcPtr);

        bool resolve(const std::string &moduleName,
                     const std::string &name,
                     IR::ExternType type,
                     Runtime::Object *&resolved) override;

        std::map<std::string, std::string> buildDisassemblyMap();

        void addFdForThisThread(int fd);

        void removeFdForThisThread(int fd);

        void clearFds();

        void checkThreadOwnsFd(int fd);

        std::string getBoundUser();

        std::string getBoundFunction();

        bool getBoundIsTypescript();

        I32 getGlobalI32(const std::string &globalName, Runtime::Context *context);

        bool tearDown();

        int getDynamicModuleCount();

        int getNextMemoryBase();

        int getNextStackPointer();

        int getNextTableBase();

        int getFunctionOffsetFromGOT(const std::string &funcName);

        int getDataOffsetFromGOT(const std::string &name);

        void writeMemoryToFd(int fd);

        void mapMemoryFromFd();

        void snapshotToFile(const std::string &filePath);

        std::vector<uint8_t> snapshotToMemory();

        size_t snapshotToState(const std::string &stateKey);

        void restoreFromFile(const std::string &filePath);

        void restoreFromMemory(const std::vector<uint8_t> &data);

        void restoreFromState(const std::string &stateKey, size_t stateSize);

        ssize_t captureStdout(const struct iovec *iovecs, int iovecCount);

        ssize_t captureStdout(const void *buffer);

        std::string getCapturedStdout();

        void clearCapturedStdout();

        I64 executeThread(WasmThreadSpec &spec);

        U32 getArgc();

        U32 getArgvBufferSize();

        void writeArgvToMemory(U32 wasmArgvPointers, U32 wasmArgvBuffer);

        void writeWasmEnvToMemory(U32 envPointers, U32 envBuffer);

        storage::FileSystem &getFileSystem();

        WasmEnvironment &getWasmEnvironment();

    private:
        Runtime::GCPointer<Runtime::Instance> envModule;
        Runtime::GCPointer<Runtime::Instance> wasiModule;
        Runtime::GCPointer<Runtime::Instance> moduleInstance;

        // Dynamic modules
        int dynamicModuleCount = 0;
        int nextMemoryBase = 0;
        int nextStackPointer = 0;
        int nextTableBase = 0;

        int memoryFd = -1;
        size_t memoryFdSize = 0;

        bool _isBound = false;
        std::string boundUser;
        std::string boundFunction;
        bool boundIsTypescript = false;

        // Shared memory regions
        std::unordered_map<std::string, I32> sharedMemWasmPtrs;

        // Map of dynamically loaded modules
        std::unordered_map<std::string, int> dynamicPathToHandleMap;
        std::unordered_map<int, Runtime::GCPointer<Runtime::Instance>> dynamicModuleMap;

        // Dynamic linking tables and memories
        std::unordered_map<std::string, Uptr> globalOffsetTableMap;
        std::unordered_map<std::string, int> globalOffsetMemoryMap;
        std::unordered_map<std::string, int> missingGlobalOffsetEntries;

        // Output buffer
        int stdoutMemFd;
        ssize_t stdoutSize;

        int getStdoutFd();

        // Argc/argv
        unsigned int argc;
        std::vector<std::string> argv;
        size_t argvBufferSize;

        void writeStringArrayToMemory(const std::vector<std::string> &strings, U32 strPoitners, U32 strBuffer);

        // Filesystem
        storage::FileSystem filesystem;

        // Environment
        WasmEnvironment wasmEnvironment;

        void doSnapshot(std::ostream &outStream);

        void doRestore(std::istream &inStream);

        void clone(const WasmModule &other);

        void addModuleToGOT(IR::Module &mod, bool isMainModule);

        void prepareArgcArgv(const message::Message &msg);

        void executeZygoteFunction();

        void executeWasmConstructorsFunction(Runtime::Instance *module);

        Runtime::Instance *createModuleInstance(
                const std::string &name,
                const std::string &sharedModulePath
        );

        Runtime::Function *getMainFunction(Runtime::Instance *module);

        Runtime::Function *getDefaultZygoteFunction(Runtime::Instance *module);

        Runtime::Function *getWasmConstructorsFunction(Runtime::Instance *module);
    };

    WasmModule *getExecutingModule();

    void setExecutingModule(WasmModule *executingModule);

    message::Message *getExecutingCall();

    void setExecutingCall(message::Message *other);

    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode) : exitCode(exitCode) {

        }

        int exitCode;
    };

    struct WasmThreadSpec {
        Runtime::ContextRuntimeData *contextRuntimeData;
        wasm::WasmModule *parentModule;
        message::Message *parentCall;
        Runtime::Function *func;
        IR::UntaggedValue *funcArgs;
        size_t stackSize;
    };
}