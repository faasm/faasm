#pragma once

#include <wasm/WasmModule.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {
    WAVM_DECLARE_INTRINSIC_MODULE(env)

    WAVM_DECLARE_INTRINSIC_MODULE(wasi)

    WAVM_DECLARE_INTRINSIC_MODULE(tsenv)

    struct WasmThreadSpec;

    class WAVMWasmModule : public WasmModule, Runtime::Resolver {
    public:
        WAVMWasmModule();

        WAVMWasmModule(const WAVMWasmModule &other);

        WAVMWasmModule &operator=(const WAVMWasmModule &other);

        ~WAVMWasmModule() override;

        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg) override;

        void bindToFunctionNoZygote(const message::Message &msg) override;

        bool execute(message::Message &msg) override;

        const bool isBound() override;

        bool tearDown();

        // ----- Memory management -----
        uint32_t mmapMemory(uint32_t length);

        uint32_t mmapPages(uint32_t pages);

        uint32_t mmapFile(uint32_t fp, uint32_t length);

        uint32_t mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length);

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) override;

        // ----- CoW memory -----
        void writeMemoryToFd(int fd) override;

        void mapMemoryFromFd() override;

        // ----- Internals -----
        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        Runtime::GCPointer<Runtime::Table> defaultTable;

        Runtime::GCPointer<Runtime::Context> executionContext;

        Runtime::GCPointer<Runtime::Compartment> compartment;

        // ----- Execution -----
        void executeFunction(
                Runtime::Function *func,
                IR::FunctionType funcType,
                const std::vector<IR::UntaggedValue> &arguments,
                IR::UntaggedValue &result
        );

        void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) override;

        // ----- Resolution/ linking -----

        Runtime::Function *getFunction(Runtime::Instance *module, const std::string &funcName, bool strict);

        Runtime::Function *getFunctionFromPtr(int funcPtr);

        bool resolve(const std::string &moduleName,
                     const std::string &name,
                     IR::ExternType type,
                     Runtime::Object *&resolved) override;

        int32_t getGlobalI32(const std::string &globalName, Runtime::Context *context);

        // ----- Threading -----
        int64_t executeThread(WasmThreadSpec &spec);

        // ----- Disassembly -----
        std::map<std::string, std::string> buildDisassemblyMap();

        // ----- Dynamic linking -----
        int dynamicLoadModule(const std::string &path, Runtime::Context *context);

        uint32_t getDynamicModuleFunction(int handle, const std::string &funcName);

        int getDynamicModuleCount();

        uint32_t addFunctionToTable(Runtime::Object *exportedFunc);

        int getNextMemoryBase();

        int getNextStackPointer();

        int getNextTableBase();

        int getFunctionOffsetFromGOT(const std::string &funcName);

        int getDataOffsetFromGOT(const std::string &name);

    protected:
        void doSnapshot(std::ostream &outStream) override;

        void doRestore(std::istream &inStream) override;

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

        void doBindToFunction(const message::Message &msg, bool executeZygote);

        void writeStringArrayToMemory(const std::vector<std::string> &strings, uint32_t strPoitners, uint32_t strBuffer);

        void clone(const WAVMWasmModule &other);

        void addModuleToGOT(IR::Module &mod, bool isMainModule);

        void executeZygoteFunction();

        void executeWasmConstructorsFunction(Runtime::Instance *module);

        Runtime::Instance *createModuleInstance(
                const std::string &name,
                const std::string &sharedModulePath
        );

        Runtime::Function *getMainFunction(Runtime::Instance *module);

        Runtime::Function *getDefaultZygoteFunction(Runtime::Instance *module);

        Runtime::Function *getWasmConstructorsFunction(Runtime::Instance *module);

        void syncPythonFunctionFile(const message::Message &msg);
    };

    WAVMWasmModule *getExecutingModule();

    void setExecutingModule(WAVMWasmModule *executingModule);

    struct WasmThreadSpec {
        Runtime::ContextRuntimeData *contextRuntimeData;
        wasm::WAVMWasmModule *parentModule;
        message::Message *parentCall;
        Runtime::Function *func;
        IR::UntaggedValue *funcArgs;
        size_t stackSize;
    };
}