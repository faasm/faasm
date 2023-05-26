#pragma once

#include <wamr/WAMRModuleMixin.h>
#include <wasm/WasmModule.h>
#include <wasm_runtime_common.h>

#include <setjmp.h>

#define ERROR_BUFFER_SIZE 256
#define STACK_SIZE_KB 8192
#define HEAP_SIZE_KB 8192

#define WAMR_INTERNAL_EXCEPTION_PREFIX "Exception: "
#define WAMR_EXIT_PREFIX "wamr_exit_code_"

namespace wasm {

enum WAMRExceptionTypes
{
    NoException = 0,
    DefaultException = 1,
    FunctionMigratedException = 2,
    QueueTimeoutException = 3,
};

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

    // ----- Exception handling -----
    void doThrowException(std::exception& e) override;

    // ----- Helper functions -----
    void writeStringToWasmMemory(const std::string& strHost, char* strWasm);

    void writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm, char* envBuffWasm);

    // ----- Address translation and validation -----

    // Check if WASM offset belongs to WASM memory
    void validateWasmOffset(uint32_t wasmOffset, size_t size);

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
    char errorBuffer[ERROR_BUFFER_SIZE];

    std::vector<uint8_t> wasmBytes;
    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;

    jmp_buf wamrExceptionJmpBuf;

    int executeWasmFunction(const std::string& funcName);

    int executeWasmFunctionFromPointer(faabric::Message& msg);

    bool executeCatchException(WASMExecEnv* execEnv,
                               WASMFunctionInstanceCommon* func,
                               int wasmFuncPtr,
                               int argc,
                               std::vector<uint32_t>& argv);

    void bindInternal(faabric::Message& msg);

    bool doGrowMemory(uint32_t pageChange) override;
};

WAMRWasmModule* getExecutingWAMRModule();
}
