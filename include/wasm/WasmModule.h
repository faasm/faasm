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
#include <memory/MemorySnapshot.h>

#define ONE_MB_BYTES 1024 * 1024

// Note: this is *not* controlling the size provisioned by the linker, that is hard-coded in the build.
// This variable is just here for reference and must be updated to match the value in the build.
#define STACK_SIZE 4 * ONE_MB_BYTES

// Properties of dynamic modules - NOTE - these MUST be passed when compiling the modules themselves
#define DYNAMIC_MODULE_STACK_SIZE 2 * ONE_MB_BYTES
#define DYNAMIC_MODULE_HEAP_SIZE 30 * ONE_MB_BYTES

// Zygote function (must match faasm.h linked into the functions themselves)
#define ZYGOTE_FUNC_NAME "_faasm_zygote"


using namespace WAVM;


namespace wasm {
    DECLARE_INTRINSIC_MODULE(env)

    Uptr getNumberOfPagesForBytes(U32 nBytes);

    class WasmModule final : Runtime::Resolver {
    public:
        WasmModule();

        WasmModule(const WasmModule &other);

        ~WasmModule();

        void initialise();

        void bindToFunction(const message::Message &msg);

        int execute(message::Message &msg);

        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        Runtime::GCPointer<Runtime::Table> defaultTable;

        Runtime::GCPointer<Runtime::Context> executionContext;

        Runtime::GCPointer<Runtime::Context> baseContext;

        std::vector<IR::Value> invokeArgs;

        Runtime::GCPointer<Runtime::Compartment> compartment;

        bool isBound();

        U32 mmap(U32 length);

        U32 mmapKey(std::shared_ptr<state::StateKeyValue> kv, U32 length);

        int dynamicLoadModule(const std::string &path, Runtime::Context *context);

        Uptr getDynamicModuleFunction(int handle, const std::string &funcName);

        int addFunctionToTable(Runtime::Object *exportedFunc);

        IR::ValueTuple executeFunction(Runtime::Function *func, const std::vector<IR::Value>& arguments);

        Runtime::Function *getFunction(const std::string &funcName, bool strict);

        void setErrno(int newValue);

        Uptr getInitialMemoryPages();

        int getHeapBase();

        int getDataEnd();

        int getStackTop();

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
    private:
        Runtime::GCPointer<Runtime::ModuleInstance> envModule;
        Runtime::GCPointer<Runtime::ModuleInstance> moduleInstance;
        Runtime::GCPointer<Runtime::Function> functionInstance;
        Runtime::GCPointer<Runtime::Function> zygoteFunctionInstance;

        // Main module
        int errnoLocation = 0;
        Uptr initialMemoryPages = 0;
        int initialTableSize = 0;
        int heapBase = 0;
        int dataEnd = 0;
        int stackTop = 0;

        // Dynamic modules
        int dynamicModuleCount = 0;
        int nextMemoryBase = 0;
        int nextStackPointer = 0;
        int nextTableBase = 0;

        bool _isBound = false;
        std::string boundUser;
        std::string boundFunction;

        // Shared memory regions
        std::unordered_map<std::string, I32> sharedMemWasmPtrs;
        std::unordered_map<std::string, void *> sharedMemHostPtrs;
        std::unordered_map<std::string, std::shared_ptr<state::StateKeyValue>> sharedMemKVs;

        // Map of dynamically loaded modules
        std::unordered_map<std::string, int> dynamicPathToHandleMap;
        std::unordered_map<int, Runtime::GCPointer<Runtime::ModuleInstance>> dynamicModuleMap;

        // Dynamic linking tables and memories
        std::unordered_map<std::string, int> globalOffsetTableMap;
        std::unordered_map<std::string, int> globalOffsetMemoryMap;
        std::unordered_map<std::string, int> missingGlobalOffsetEntries;

        WasmModule &operator=(const WasmModule &other);

        void reset();

        void resizeMemory(size_t targetPages);

        void addModuleToGOT(IR::Module &mod, bool isMainModule);

        Runtime::ModuleInstance *createModuleInstance(
                const std::string &name,
                const std::string &sharedModulePath
        );

        I32 getGlobalI32(const std::string &globalName, Runtime::Context *context);
    };

    WasmModule *getExecutingModule();

    message::Message *getExecutingCall();

    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode) : exitCode(exitCode) {

        }

        int exitCode;
    };

    class WasmModuleRegistry {
    public:
        WasmModuleRegistry();

        void registerModule(const std::string &key, const WasmModule &module);

        WasmModule &getModule(const std::string &key);
    private:
        std::mutex registryMutex;
        std::unordered_map<std::string, WasmModule> moduleMap;
    };

    WasmModuleRegistry &getWasmModuleRegistry();
}