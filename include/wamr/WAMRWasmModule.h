#pragma once

#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 1024
#define HEAP_SIZE_KB 1024

namespace wasm {
    class WAMRWasmModule final : public WasmModule {
    public:
        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg);

        void bindToFunctionNoZygote(const message::Message &msg);

        bool execute(message::Message &msg, bool forceNoop = false);

        const bool isBound();

        void tearDown();

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer);

        // ----- CoW memory -----
        void writeMemoryToFd(int fd);

        void mapMemoryFromFd();

        // ----- Argc/argv -----
        void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer);

    protected:
        void doSnapshot(std::ostream &outStream);

        void doRestore(std::istream &inStream);

    private:
        bool _isBound;

        std::vector<uint8_t> wasmFileBytes;
        std::vector<char> errorBuffer;

        WASMModuleCommon *wasmModule;
        WASMModuleInstanceCommon *moduleInstance;
        WASMExecEnv *executionEnv;

        void executeFunction(const std::string &funcName);
    };
}