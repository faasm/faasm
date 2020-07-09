#pragma once

#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 1024
#define HEAP_SIZE_KB 1024

namespace wasm {
    class WAMRWasmModule final : public WasmModule {
    public:
        ~WAMRWasmModule();

        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg) override;

        void bindToFunctionNoZygote(const message::Message &msg) override;

        bool execute(message::Message &msg, bool forceNoop = false) override;

        const bool isBound() override;

        void tearDown();
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