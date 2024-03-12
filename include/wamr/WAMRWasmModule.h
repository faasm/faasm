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

class WAMRWasmModule;

#define WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED(str)                                \
    auto __errorStr = fmt::format("{} not implemented!", str);                 \
    SPDLOG_ERROR(__errorStr);                                                  \
    auto __ex = std::runtime_error(__errorStr);                                \
    auto __module = wasm::getExecutingWAMRModule();                            \
    __module->doThrowException(__ex);

enum WAMRExceptionTypes
{
    NoException = 0,
    DefaultException = 1,
    FunctionMigratedException = 2,
    QueueTimeoutException = 3,
};

std::vector<uint8_t> wamrCodegen(std::vector<uint8_t>& wasmBytesIn, bool isSgx);

class WAMRWasmModule final
  : public WasmModule
  , public WAMRModuleMixin<WAMRWasmModule>
{
  public:
    static void initialiseWAMRGlobally();

    static void destroyWAMRGlobally();

    WAMRWasmModule();

    explicit WAMRWasmModule(int threadPoolSizeIn);

    ~WAMRWasmModule();

    // ----- Module lifecycle -----
    void reset(faabric::Message& msg, const std::string& snapshotKey) override;

    void doBindToFunction(faabric::Message& msg, bool cache) override;

    int32_t executeFunction(faabric::Message& msg) override;

    // ----- Threads ------
    int32_t executeOMPThread(int threadPoolIdx,
                             uint32_t stackTop,
                             faabric::Message& msg) override;

    int32_t executePthread(int threadPoolIdx,
                           uint32_t stackTop,
                           faabric::Message& msg) override;

    void createThreadsExecEnv(WASMExecEnv* parentExecEnv);

    void destroyThreadsExecEnv(bool destroyMainExecEnv = false);

    // ----- Exception handling -----
    void doThrowException(std::exception& e) override;

    // ----- Helper functions -----
    void writeStringToWasmMemory(const std::string& strHost, char* strWasm);

    void writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm, char* envBuffWasm);

    // ----- Address translation and validation -----
    // Check if native pointer belongs to WASM memory
    void validateNativePointer(void* nativePtr, int size);

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
    // WAMR's execution environments are not thread-safe. Thus, we create an
    // array of them at the beginning, each thread will access a different
    // position in the array, so we do not need a mutex
    std::vector<WASMExecEnv*> execEnvs;

    jmp_buf wamrExceptionJmpBuf;

    int executeWasmFunction(int threadPoolIdx, const std::string& funcName);

    int executeWasmFunctionFromPointer(int threadPoolIdx,
                                       faabric::Message& msg);

    bool executeCatchException(int threadPoolIdx,
                               WASMFunctionInstanceCommon* func,
                               int wasmFuncPtr,
                               int argc,
                               std::vector<uint32_t>& argv);

    void bindInternal(faabric::Message& msg);

    bool doGrowMemory(uint32_t pageChange) override;
};

WAMRWasmModule* getExecutingWAMRModule();
}
