#pragma once

#include <wasm_runtime_common.h>
#include "WasmModule.h"

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 1024
#define HEAP_SIZE_KB 1024

namespace wasm {
    class WAMRWasmModule: public WasmModule {
    public:
        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg) override;

        void bindToFunctionNoZygote(const message::Message &msg) override;

        bool execute(message::Message &msg) override;

        const bool isBound() override;

        void tearDown();

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) override;

        // ----- CoW memory -----
        void writeMemoryToFd(int fd) override;

        void mapMemoryFromFd() override;

        // ----- Argc/argv -----
        void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) override;
    protected:
        void doSnapshot(std::ostream &outStream) override;

        void doRestore(std::istream &inStream) override;
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