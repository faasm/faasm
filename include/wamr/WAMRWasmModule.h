#pragma once

#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 8192
#define HEAP_SIZE_KB 8192

// Address translation macros

#define VALIDATE_NATIVE_ADDR(ptr, size)                                        \
    if (!wasm_runtime_validate_native_addr(                                    \
          module->getModuleInstance(), ptr, size)) {                           \
        return __WASI_EBADF;                                                   \
    }

#define VALIDATE_APP_ADDR(ptr, size)                                           \
    if (!wasm_runtime_validate_app_addr(                                       \
          module->getModuleInstance(), ptr, size)) {                           \
        return __WASI_EBADF;                                                   \
    }

namespace wasm {

std::vector<uint8_t> wamrCodegen(std::vector<uint8_t>& wasmBytes, bool isSgx);

class WAMRWasmModule final : public WasmModule
{
  public:
    static void initialiseWAMRGlobally();

    WAMRWasmModule();

    explicit WAMRWasmModule(int threadPoolSizeIn);

    ~WAMRWasmModule();

    // ----- Module lifecycle -----
    void doBindToFunction(faabric::Message& msg, bool cache) override;

    int32_t executeFunction(faabric::Message& msg) override;

    // ----- Memory management -----
    uint32_t growMemory(uint32_t nBytes) override;

    uint32_t shrinkMemory(uint32_t nBytes) override;

    uint32_t mmapMemory(uint32_t nBytes) override;

    uint32_t mmapFile(uint32_t fp, uint32_t length) override;

    uint8_t* wasmPointerToNative(int32_t wasmPtr) override;

    int32_t nativePointerToWasm(uint8_t* nativePtr);

    size_t getMemorySizeBytes() override;

    WASMModuleInstanceCommon* getModuleInstance();

  private:
    char errorBuffer[ERROR_BUFFER_SIZE];

    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;

    int executeWasmFunction(const std::string& funcName);

    int executeWasmFunctionFromPointer(int wasmFuncPtr);
};

WAMRWasmModule* getExecutingWAMRModule();

void tearDownWAMRGlobally();
}
