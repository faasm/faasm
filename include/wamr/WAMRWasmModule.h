#pragma once

#include <wamr/WAMRModuleMixin.h>
#include <wasm/WasmCommon.h>
#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

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
    void writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm, char* envBuffWasm);

    // ----- Address translation and validation -----

    // Convert relative address to absolute address (pointer to memory)
    uint8_t* wasmPointerToNative(uint32_t wasmPtr) override;

    // ----- Memory management -----
    uint32_t mmapFile(uint32_t fp, size_t length) override;

    size_t getMemorySizeBytes() override;

    uint8_t* getMemoryBase() override;

    size_t getMaxMemoryPages() override;

    WASMModuleInstanceCommon* getModuleInstance();

    std::vector<std::string> getArgv();

  private:
    char errorBuffer[WAMR_ERROR_BUFFER_SIZE];

    std::vector<uint8_t> wasmBytes;
    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;

    int executeWasmFunction(const std::string& funcName);

    int executeWasmFunctionFromPointer(int wasmFuncPtr);

    void bindInternal(faabric::Message& msg);

    bool doGrowMemory(uint32_t pageChange) override;
};

WAMRWasmModule* getExecutingWAMRModule();
}
