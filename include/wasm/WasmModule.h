#pragma once

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

#define ONE_MB_BYTES 1024 * 1024

// Note: this is *not* controlling the size provisioned by the linker, that is hard-coded in the build.
// This variable is just here for reference and must be updated to match the value in the build.
#define STACK_SIZE 4 * ONE_MB_BYTES

// Properties of dynamic modules - NOTE - these MUST be passed when compiling the modules themselves
// Heap size must be wasm-module-page-aligned. One page is 64kB so 480 pages is 30MB
#define DYNAMIC_MODULE_STACK_SIZE 2 * ONE_MB_BYTES
#define DYNAMIC_MODULE_HEAP_PAGES 480

// Zygote function (must match faasm.h linked into the functions themselves)
#define ZYGOTE_FUNC_NAME "_faasm_zygote"
#define ENTRY_FUNC_NAME "main"


using namespace WAVM;


namespace wasm {
    WAVM_DECLARE_INTRINSIC_MODULE(env)

    WAVM_DECLARE_INTRINSIC_MODULE(tsenv)

    Uptr getNumberOfPagesForBytes(U32 nBytes);

    class WasmModule final : Runtime::Resolver {
    public:
        WasmModule();

        WasmModule(const WasmModule &other);

        WasmModule &operator=(const WasmModule &other);

        ~WasmModule();

        void bindToFunction(const message::Message &msg);

        int execute(message::Message &msg);

        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        Runtime::GCPointer<Runtime::Table> defaultTable;

        Runtime::GCPointer<Runtime::Context> executionContext;

        std::vector<IR::UntaggedValue> invokeArgs;

        Runtime::GCPointer<Runtime::Compartment> compartment;

        const bool isBound();

        U32 mmapMemory(U32 length);

        U32 mmapPages(U32 pages);

        U32 mmapFile(U32 fp, U32 length);

        U32 mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, U32 length);

        int dynamicLoadModule(const std::string &path, Runtime::Context *context);

        Runtime::ModuleInstance *getDynamicModule(int handle);

        Uptr getDynamicModuleFunction(int handle, const std::string &funcName);

        Uptr addFunctionToTable(Runtime::Object *exportedFunc);

        void executeFunction(
                Runtime::Function *func,
                IR::FunctionType funcType,
                const std::vector<IR::UntaggedValue> &arguments,
                IR::UntaggedValue &result
        );

        Runtime::Function *getFunction(const std::string &funcName, bool strict);

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

    private:
        Runtime::GCPointer<Runtime::ModuleInstance> envModule;
        Runtime::GCPointer<Runtime::ModuleInstance> moduleInstance;

        // Note: we don't use GCPointers for the function instances as there are tied
        // to the lifecycle of the underlying module
        Runtime::Function *functionInstance;
        Runtime::Function *zygoteFunctionInstance;

        // Dynamic modules
        int dynamicModuleCount = 0;
        int nextMemoryBase = 0;
        int nextStackPointer = 0;
        int nextTableBase = 0;

        bool _isBound = false;
        std::string boundUser;
        std::string boundFunction;
        bool boundIsTypescript = false;

        // Shared memory regions
        std::unordered_map<std::string, I32> sharedMemWasmPtrs;

        // Map of dynamically loaded modules
        std::unordered_map<std::string, int> dynamicPathToHandleMap;
        std::unordered_map<int, Runtime::GCPointer<Runtime::ModuleInstance>> dynamicModuleMap;

        // Dynamic linking tables and memories
        std::unordered_map<std::string, Uptr> globalOffsetTableMap;
        std::unordered_map<std::string, int> globalOffsetMemoryMap;
        std::unordered_map<std::string, int> missingGlobalOffsetEntries;

        void reset();

        void clone(const WasmModule &other);

        void addModuleToGOT(IR::Module &mod, bool isMainModule);

        Runtime::ModuleInstance *createModuleInstance(
                const std::string &name,
                const std::string &sharedModulePath
        );
    };

    WasmModule *getExecutingModule();

    message::Message *getExecutingCall();

    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode) : exitCode(exitCode) {

        }

        int exitCode;
    };
}