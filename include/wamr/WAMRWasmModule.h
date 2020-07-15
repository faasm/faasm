#pragma once

#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 1024
#define HEAP_SIZE_KB 1024
#define WASM_BYTES_PER_PAGE 65536


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

        // ----- Memory management -----
        uint32_t mmapMemory(uint32_t length) override;

        uint32_t mmapPages(uint32_t pages) override;

        uint32_t mmapFile(uint32_t fp, uint32_t length) override;

        uint8_t* wasmPointerToNative(int32_t wasmPtr) override;
    private:
        bool _isBound;

        std::vector<char> errorBuffer;

        WASMModuleCommon *wasmModule;
        WASMModuleInstanceCommon *moduleInstance;
        WASMExecEnv *executionEnv;

        void executeFunction(const std::string &funcName);
    };

    void initialiseWAMRGlobally();

    void tearDownWAMRGlobally();

    WAMRWasmModule *getExecutingWAMRModule();

    void setExecutingModule(WAMRWasmModule *executingModuleIn);
}