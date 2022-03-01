#pragma once

#include <iwasm/aot/aot_runtime.h>
#include <wasm_runtime_common.h>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#define ONE_KB_BYTES 1024
#define ONE_MB_BYTES (1024 * 1024)

#define FAASM_SGX_WAMR_HEAP_SIZE (32 * ONE_MB_BYTES)
#define FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE 128
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE (8 * ONE_KB_BYTES)
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE (8 * ONE_KB_BYTES)

#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define WASM_ENTRY_FUNC "_start"

namespace wasm {

/*
 * Abstraction around a WebAssembly module running inside an SGX enclave with
 * the WAMR runtime.  */
class EnclaveWasmModule
{
  public:
    static bool initialiseWAMRGlobally();

    EnclaveWasmModule();

    ~EnclaveWasmModule();

    bool loadWasm(void* wasmOpCodePtr, uint32_t wasmOpCodeSize);

    bool callFunction(uint32_t argcIn, char** argvIn);

    bool validateNativeAddress(void* nativePtr, size_t size);

    WASMModuleInstanceCommon* moduleInstance;

    // ---- argc/arv ----
    uint32_t getArgc();

    size_t getArgvBufferSize();

    void writeArgvToWamrMemory(uint32_t wasmArgvPointers,
                               uint32_t wasmArgvBuffer);

  private:
    char errorBuffer[FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE];

    WASMModuleCommon* wasmModule;

    // Argc/argv
    uint32_t argc;
    std::vector<std::string> argv;
    size_t argvBufferSize;

    void prepareArgcArgv(uint32_t argcIn, char** argvIn);
};

/*
 * Data structure to keep track of the modules currently loaded in the enclave.
 * And mutex to control concurrent accesses. Both objects have external
 * definition as they have to be accessed both when running an ECall, and
 * resolving a WAMR native symbol.
 */
extern std::unordered_map<uint32_t, std::shared_ptr<wasm::EnclaveWasmModule>>
  moduleMap;
extern std::mutex moduleMapMutex;

std::shared_ptr<wasm::EnclaveWasmModule> getExecutingEnclaveWasmModule(
  wasm_exec_env_t execEnv);
}
