#pragma once

#include <wasm/WasmModule.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>

namespace wasm {
    WAVM_DECLARE_INTRINSIC_MODULE(env)

    WAVM_DECLARE_INTRINSIC_MODULE(wasi)

    WAVM_DECLARE_INTRINSIC_MODULE(tsenv)

    struct WasmThreadSpec;

    namespace openmp {
        class PlatformThreadPool;
    }

    class WAVMWasmModule final : public WasmModule, WAVM::Runtime::Resolver {
    public:
        WAVMWasmModule();

        WAVMWasmModule(const WAVMWasmModule &other);

        WAVMWasmModule &operator=(const WAVMWasmModule &other);

        ~WAVMWasmModule();

        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg);

        void bindToFunctionNoZygote(const message::Message &msg);

        bool execute(message::Message &msg, bool forceNoop = false);

        const bool isBound();

        bool tearDown();

        // ----- Memory management -----
        uint32_t mmapMemory(uint32_t length);

        uint32_t mmapPages(uint32_t pages);

        uint32_t mmapFile(uint32_t fp, uint32_t length);

        uint32_t mapSharedStateMemory(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length);

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer);

        // ----- CoW memory -----
        void writeMemoryToFd(int fd);

        void mapMemoryFromFd();

        // ----- Internals -----
        WAVM::Runtime::GCPointer<WAVM::Runtime::Memory> defaultMemory;

        WAVM::Runtime::GCPointer<WAVM::Runtime::Table> defaultTable;

        WAVM::Runtime::GCPointer<WAVM::Runtime::Context> executionContext;

        WAVM::Runtime::GCPointer<WAVM::Runtime::Compartment> compartment;

        // ----- Execution -----
        void executeFunction(
                WAVM::Runtime::Function *func,
                WAVM::IR::FunctionType funcType,
                const std::vector<WAVM::IR::UntaggedValue> &arguments,
                WAVM::IR::UntaggedValue &result
        );

        void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer);

        // ----- Resolution/ linking -----

        WAVM::Runtime::Function *getFunction(WAVM::Runtime::Instance *module, const std::string &funcName, bool strict);

        WAVM::Runtime::Function *getFunctionFromPtr(int funcPtr);

        bool resolve(const std::string &moduleName,
                     const std::string &name,
                     WAVM::IR::ExternType type,
                     WAVM::Runtime::Object *&resolved);

        int32_t getGlobalI32(const std::string &globalName, WAVM::Runtime::Context *context);

        // ----- Threading -----
        int64_t executeThreadLocally(WasmThreadSpec &spec);

        // ----- Disassembly -----
        std::map<std::string, std::string> buildDisassemblyMap();

        // ----- Dynamic linking -----
        int dynamicLoadModule(const std::string &path, WAVM::Runtime::Context *context);

        uint32_t getDynamicModuleFunction(int handle, const std::string &funcName);

        int getDynamicModuleCount();

        uint32_t addFunctionToTable(WAVM::Runtime::Object *exportedFunc);

        int getNextMemoryBase();

        int getNextStackPointer();

        int getNextTableBase();

        int getFunctionOffsetFromGOT(const std::string &funcName);

        int getDataOffsetFromGOT(const std::string &name);

        U32 allocateThreadStack();

        std::unique_ptr<openmp::PlatformThreadPool> &getOMPPool();

    protected:
        void doSnapshot(std::ostream &outStream);

        void doRestore(std::istream &inStream);

    private:
        WAVM::Runtime::GCPointer<WAVM::Runtime::Instance> envModule;
        WAVM::Runtime::GCPointer<WAVM::Runtime::Instance> wasiModule;
        WAVM::Runtime::GCPointer<WAVM::Runtime::Instance> moduleInstance;

        // Dynamic modules
        int dynamicModuleCount = 0;
        int nextMemoryBase = 0;
        int nextStackPointer = 0;
        int nextTableBase = 0;

        int memoryFd = -1;
        size_t memoryFdSize = 0;

        bool _isBound = false;
        bool boundIsTypescript = false;

        // Shared memory regions
        std::mutex sharedMemWasmPtrsMx;
        std::unordered_map<std::string, U32> sharedMemWasmPtrs;

        // Map of dynamically loaded modules
        std::unordered_map<std::string, int> dynamicPathToHandleMap;
        std::unordered_map<int, WAVM::Runtime::GCPointer<WAVM::Runtime::Instance>> dynamicModuleMap;

        // Dynamic linking tables and memories
        std::unordered_map<std::string, WAVM::Uptr> globalOffsetTableMap;
        std::unordered_map<std::string, int> globalOffsetMemoryMap;
        std::unordered_map<std::string, int> missingGlobalOffsetEntries;

        static WAVM::Runtime::Instance *getEnvModule();

        static WAVM::Runtime::Instance *getWasiModule();

        void doBindToFunction(const message::Message &msg, bool executeZygote);

        void writeStringArrayToMemory(
                const std::vector<std::string> &strings, uint32_t strPoitners, uint32_t strBuffer);

        void clone(const WAVMWasmModule &other);

        void addModuleToGOT(WAVM::IR::Module &mod, bool isMainModule);

        void executeZygoteFunction();

        void executeWasmConstructorsFunction(WAVM::Runtime::Instance *module);

        WAVM::Runtime::Instance *createModuleInstance(
                const std::string &name,
                const std::string &sharedModulePath
        );

        WAVM::Runtime::Function *getMainFunction(WAVM::Runtime::Instance *module);

        WAVM::Runtime::Function *getDefaultZygoteFunction(WAVM::Runtime::Instance *module);

        WAVM::Runtime::Function *getWasmConstructorsFunction(WAVM::Runtime::Instance *module);

        void syncPythonFunctionFile(const message::Message &msg);

        void executeRemoteOMP(message::Message &msg);

        void prepareOpenMPContext(const message::Message &msg);

        std::unique_ptr<openmp::PlatformThreadPool> OMPPool;
    };

    WAVMWasmModule *getExecutingModule();

    void setExecutingModule(WAVMWasmModule *executingModule);

    struct WasmThreadSpec {
        WAVM::Runtime::ContextRuntimeData *contextRuntimeData;
        WAVM::Runtime::Function *func;
        WAVM::IR::UntaggedValue *funcArgs;
        U32 stackTop;
    };
}
