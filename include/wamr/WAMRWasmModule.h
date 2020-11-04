#pragma once

#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 8192
#define HEAP_SIZE_KB 8192
#define WASM_BYTES_PER_PAGE 65536

namespace wasm {
#if (!WAMR_EXECUTION_MODE_INTERP)
std::vector<uint8_t> wamrCodegen(std::vector<uint8_t>& wasmBytes);
#endif
class WAMRWasmModule final : public WasmModule
{
  public:
    static void initialiseWAMRGlobally();

    WAMRWasmModule();

    ~WAMRWasmModule();

    // ----- Module lifecycle -----
    void bindToFunction(const faabric::Message& msg) override;

    void bindToFunctionNoZygote(const faabric::Message& msg) override;

    bool execute(faabric::Message& msg, bool forceNoop = false) override;

    bool isBound() override;

    void tearDown();

    // ----- Memory management -----
    uint32_t mmapMemory(uint32_t length) override;

    uint32_t mmapPages(uint32_t pages) override;

    uint32_t mmapFile(uint32_t fp, uint32_t length) override;

    uint8_t* wasmPointerToNative(int32_t wasmPtr) override;

  private:
    bool _isBound;

    char errorBuffer[ERROR_BUFFER_SIZE];

#if (WAMR_EXECUTION_MODE_INTERP)
    std::vector<uint8_t> wasmBytes;
#endif

    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;

    void executeFunction(const std::string& funcName);
};

void tearDownWAMRGlobally();

WAMRWasmModule* getExecutingWAMRModule();

void setExecutingModule(WAMRWasmModule* executingModuleIn);
}
