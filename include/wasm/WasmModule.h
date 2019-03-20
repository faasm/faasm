#pragma once

#include "CallChain.h"
#include "RootResolver.h"

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

#define EXPAND_TOO_BIG -2
#define EXPAND_NO_ACTION -1
#define EXPAND_SUCCESS 0

using namespace WAVM;

namespace wasm {
    // This is the number of pages we copy and restore for each reuse of the module.
    const int CLEAN_MEMORY_PAGES = 1;
    const int CLEAN_MEMORY_SIZE = CLEAN_MEMORY_PAGES * IR::numBytesPerPage;

    int expandMemory(U32 newSize);

    U32 dynamicAllocString(Runtime::Memory *memory, const char *str, U32 len);

    U32 dynamicAlloc(Runtime::Memory *memory, U32 numBytes);

    void setEmscriptenDynamicTop(U32 newValue);

    U32 getEmscriptenDynamicTop();

    Uptr getNumberOfPagesForBytes(U32 nBytes);

    class WasmModule {
    public:
        WasmModule();

        ~WasmModule();

        void initialise();

        void bindToFunction(const message::Message &msg);

        int execute(message::Message &msg, CallChain &callChain);

        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        Runtime::GCPointer<Runtime::Compartment> compartment;

        bool isInitialised();

        bool isBound();

        bool isEmscripten();

        U32 mmap(U32 length);

        U32 mmapKey(std::shared_ptr<state::StateKeyValue> kv, U32 length);

        void snapshotFullMemory(const char *key);

        void restoreFullMemory(const char *key);

        void restoreMemory();

        int dynamicLoadModule(const std::string &path);

        Runtime::Function *getFunction(const std::string funcName);

    private:
        IR::Module module;

        int nextHandle = 1;

        Runtime::GCPointer<Runtime::ModuleInstance> moduleInstance;
        Runtime::GCPointer<Runtime::Function> functionInstance;

        RootResolver *resolver = nullptr;

        bool _isInitialised = false;
        bool _isBound = false;
        std::string boundUser;
        std::string boundFunction;

        std::unordered_map<std::string, I32> sharedMemWasmPtrs;
        std::unordered_map<std::string, void *> sharedMemHostPtrs;
        std::unordered_map<std::string, std::shared_ptr<state::StateKeyValue>> sharedMemKVs;

        memory::MemorySnapshot memSnapshot;

        void resizeMemory(size_t targetPages);

        Runtime::ModuleInstance *
        createModuleInstance(
                IR::Module &irModule,
                const std::vector<uint8_t> &wasmBytes,
                const std::vector<uint8_t> &objBytes,
                const std::string &name
        );
    };

    WasmModule *getExecutingModule();

    message::Message *getExecutingCall();

    CallChain *getExecutingCallChain();

    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode) : exitCode(exitCode) {

        }

        int exitCode;
    };
}