#pragma once

#include "WasmModule.h"

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
        void bindToFunction(const message::Message &msg, bool executeZygote) override;

        bool execute(message::Message &msg) override;

        const bool isBound() override;

        std::string getBoundUser() override;

        std::string getBoundFunction() override;

        bool tearDown();

        // ----- Memory management -----
        uint32_t mmapMemory(uint32_t length) override;

        uint32_t mmapPages(uint32_t pages) override;

        uint32_t mmapFile(uint32_t fp, uint32_t length) override;

        uint32_t mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length) override;

        // ----- Legacy argc/ argv -----
        uint32_t getArgc() override;

        uint32_t getArgvBufferSize() override;

        void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) override;

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) override;

        // ----- Stdout capture -----
        ssize_t captureStdout(const struct iovec *iovecs, int iovecCount) override;

        ssize_t captureStdout(const void *buffer) override;

        std::string getCapturedStdout() override;

        void clearCapturedStdout() override;

        // ----- Pre-WASI filesystem stuff -----
        void addFdForThisThread(int fd) override;

        void removeFdForThisThread(int fd) override;

        void clearFds() override;

        void checkThreadOwnsFd(int fd) override;

        // ----- Typescript -----
        bool getBoundIsTypescript() override;

        // ----- CoW memory -----
        void writeMemoryToFd(int fd) override;

        void mapMemoryFromFd() override;

        // ----- Snapshot/ restore -----
        void snapshotToFile(const std::string &filePath) override;

        std::vector<uint8_t> snapshotToMemory() override;

        size_t snapshotToState(const std::string &stateKey) override;

        void restoreFromFile(const std::string &filePath) override;

        void restoreFromMemory(const std::vector<uint8_t> &data) override;

        void restoreFromState(const std::string &stateKey, size_t stateSize) override;

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

        // Output buffer
        int stdoutMemFd;
        ssize_t stdoutSize;

        int getStdoutFd();

        // Argc/argv
        unsigned int argc;
        std::vector<std::string> argv;
        size_t argvBufferSize;

        void writeStringArrayToMemory(const std::vector<std::string> &strings, uint32_t strPoitners, uint32_t strBuffer);

        void doSnapshot(std::ostream &outStream);

        void doRestore(std::istream &inStream);

        void clone(const WAVMWasmModule &other);

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

    struct WasmThreadSpec {
        Runtime::ContextRuntimeData *contextRuntimeData;
        wasm::WasmModule *parentModule;
        message::Message *parentCall;
        Runtime::Function *func;
        IR::UntaggedValue *funcArgs;
        size_t stackSize;
    };
}