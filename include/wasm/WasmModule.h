#pragma once

#include <util/logging.h>
#include <state/State.h>
#include <proto/faasm.pb.h>

#include <exception>
#include <string>
#include <tuple>
#include <thread>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>
#include <memory/MemorySnapshot.h>

// Note that page size in wasm is 64kiB
// Note also that this initial memory must be big enough to include all data, stack and dynamic
// memory that the module will need.
#define ONE_MB_PAGES 16
#define ONE_GB_PAGES 1024 * ONE_MB_PAGES
#define ONE_MB_BYTES 1024 * 1024

// Note: this is *not* controlling the size provisioned by the linker, that is hard-coded in the build.
// This variable is just here for reference and must be updated to match the value in the build.
#define STACK_SIZE 4 * ONE_MB_BYTES

#define MAX_MEMORY_PAGES ONE_GB_PAGES

// Properties of dynamic modules - NOTE - these MUST be passed when compiling the modules themselves
#define DYNAMIC_MODULE_STACK_SIZE 2 * ONE_MB_BYTES
#define DYNAMIC_MODULE_HEAP_SIZE 30 * ONE_MB_BYTES

#define MAX_TABLE_SIZE 500000

using namespace WAVM;


namespace wasm {
    DECLARE_INTRINSIC_MODULE(env)

    Uptr getNumberOfPagesForBytes(U32 nBytes);

    class WasmModule final : Runtime::Resolver {
    public:
        WasmModule();

        ~WasmModule();

        void initialise();

        void bindToFunction(const message::Message &msg);

        int execute(message::Message &msg);

        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        Runtime::GCPointer<Runtime::Table> defaultTable;

        Runtime::GCPointer<Runtime::Compartment> compartment;

        bool isInitialised();

        bool isBound();

        U32 mmap(U32 length);

        U32 mmapKey(std::shared_ptr<state::StateKeyValue> kv, U32 length);

        void snapshotFullMemory(const char* key);

        void restoreFullMemory(const char* key);

        void resetDynamicModules();

        int dynamicLoadModule(const std::string &path, Runtime::Context *context);

        Uptr getDynamicModuleFunction(int handle, const std::string &funcName);

        int addFunctionToTable(Runtime::Object *exportedFunc);

        Runtime::Function *getFunction(const std::string &funcName);

        void setErrno(int newValue);

        int getInitialMemoryPages();

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

    private:
        IR::Module module;

        Runtime::GCPointer<Runtime::ModuleInstance> envModule;

        int errnoLocation = 0;

        int dynamicModuleCount = 0;

        int initialMemoryPages = 0;
        int initialTableSize = 0;
        int heapBase = 0;
        int dataEnd = 0;
        int stackTop = 0;
        int nextMemoryBase = 0;
        int nextStackPointer = 0;
        int nextTableBase = 0;

        Runtime::GCPointer<Runtime::ModuleInstance> moduleInstance;
        Runtime::GCPointer<Runtime::Function> functionInstance;

        bool _isInitialised = false;
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

        // Dynamic linking stuff
        std::unordered_map<std::string, int> globalOffsetTableMap;
        std::unordered_map<std::string, int> globalOffsetMemoryMap;
        std::unordered_map<std::string, int> missingGlobalOffsetEntries;

        void resizeMemory(size_t targetPages);

        void addModuleToGOT(IR::Module &mod, bool isMainModule);

        Runtime::ModuleInstance *createModuleInstance(
                IR::Module &irModule,
                const std::vector<uint8_t> &wasmBytes,
                const std::vector<uint8_t> &objBytes,
                const std::string &name,
                bool isMainModule
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
}