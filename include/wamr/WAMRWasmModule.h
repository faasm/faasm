#pragma once

#include <wamr/WAMRModuleMixin.h>
#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 8192
#define HEAP_SIZE_KB 8192

#define WAMR_INTERNAL_EXCEPTION_PREFIX "Exception: "
#define WAMR_EXIT_PREFIX "wamr_exit_code_"

namespace wasm {

std::vector<uint8_t> wamrCodegen(std::vector<uint8_t>& wasmBytes, bool isSgx);

class WAMRWasmModule final
  : public WasmModule
  , public WAMRModuleMixin<WAMRWasmModule>
{
  public:
    static void initialiseWAMRGlobally();

    WAMRWasmModule();

    explicit WAMRWasmModule(int threadPoolSizeIn);

    ~WAMRWasmModule();

    // ----- Module lifecycle -----
    void reset(faabric::Message& msg, const std::string& snapshotKey) override;

    void doBindToFunction(faabric::Message& msg, bool cache) override;

    int32_t executeFunction(faabric::Message& msg) override;

    // ----- Helper functions -----
    void writeStringToWasmMemory(const std::string& strHost, char* strWasm);

    void writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm, char* envBuffWasm);

    // ----- Address translation and validation -----

    // Check if WASM offset belongs to WASM memory
    void validateWasmOffset(uint32_t wasmOffset, size_t size);

    // Convert relative address to absolute address (pointer to memory)
    uint8_t* wasmPointerToNative(uint32_t wasmPtr) override;

    // ----- Memory management -----
    uint32_t growMemory(size_t nBytes) override;

    uint32_t shrinkMemory(size_t nBytes) override;

    uint32_t mmapMemory(size_t nBytes) override;

    uint32_t mmapFile(uint32_t fp, size_t length) override;

    size_t getMemorySizeBytes() override;

    uint8_t* getMemoryBase() override;

    size_t getMaxMemoryPages();

    WASMModuleInstanceCommon* getModuleInstance();

    std::vector<std::string> getArgv();

  private:
    char errorBuffer[ERROR_BUFFER_SIZE];

    std::vector<uint8_t> wasmBytes;
    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;

    int executeWasmFunction(const std::string& funcName);

    int executeWasmFunctionFromPointer(int wasmFuncPtr);

    void bindInternal(faabric::Message& msg);
};

WAMRWasmModule* getExecutingWAMRModule();
}
