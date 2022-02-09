#pragma once

#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 8192
#define HEAP_SIZE_KB 8192

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

    // ----- Helper functions -----
    void writeStringToWasmMemory(const std::string& strHost, char* strWasm);

    void writeStringArrayToMemory(const std::vector<std::string>& strings,
                                  uint32_t* strOffsets,
                                  char* strBuffer);

    void writeArgvToWamrMemory(uint32_t* argvOffsetsWasm, char* argvBuffWasm);

    void writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm, char* envBuffWasm);

    // ----- Address translation and validation -----
    // Validate that the native address belongs to the module's instance address
    // space
    void validateNativeAddress(void* nativePtr, size_t size);

    // Check if WASM offset belongs to WASM memory
    void validateWasmOffset(uint32_t wasmOffset, size_t size);

    // Convert relative address to absolute address (pointer to memory)
    uint8_t* wasmPointerToNative(uint32_t wasmPtr) override;

    // Convert absolute address to relative address (offset in WASM memory)
    uint32_t nativePointerToWasm(void* nativePtr);

    // ----- Memory management -----
    uint32_t growMemory(size_t nBytes) override;

    uint32_t shrinkMemory(size_t nBytes) override;

    uint32_t mmapMemory(size_t nBytes) override;

    uint32_t mmapFile(uint32_t fp, size_t length) override;

    size_t getMemorySizeBytes() override;

    uint8_t* getMemoryBase() override;

    size_t getMaxMemoryPages();

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
