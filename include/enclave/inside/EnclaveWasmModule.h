#pragma once

#include <iwasm/aot/aot_runtime.h>
#include <wasm_runtime_common.h>

#define ONE_KB_BYTES 1024
#define ONE_MB_BYTES (1024 * 1024)

#define FAASM_SGX_WAMR_HEAP_SIZE (32 * ONE_MB_BYTES)
#define FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE 128
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE (8 * ONE_KB_BYTES)
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE (8 * ONE_KB_BYTES)

#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define WASM_ENTRY_FUNC "_start"

namespace wasm {

static uint8_t wamrHeapBuffer[FAASM_SGX_WAMR_HEAP_SIZE];

/*
 * Abstraction around a WebAssembly module running inside an SGX enclave with
 * the WAMR runtime.
 */
class EnclaveWasmModule
{
  public:
    static bool initialiseWAMRGlobally();

    EnclaveWasmModule();

    ~EnclaveWasmModule();

    bool loadWasm(void* wasmOpCodePtr, uint32_t wasmOpCodeSize);

    bool callFunction();

  private:
    char errorBuffer[FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE];

    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;
};
}
