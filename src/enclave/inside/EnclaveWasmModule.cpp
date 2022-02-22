#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/ocalls.h>

#include <string>
#include <vector>

namespace wasm {

bool EnclaveWasmModule::initialiseWAMRGlobally()
{
    // Initialise the WAMR runtime
    RuntimeInitArgs wamrRteArgs;
    memset(&wamrRteArgs, 0x0, sizeof(wamrRteArgs));
    wamrRteArgs.mem_alloc_type = Alloc_With_Pool;
    wamrRteArgs.mem_alloc_option.pool.heap_buf = (void*)wamrHeapBuffer;
    wamrRteArgs.mem_alloc_option.pool.heap_size = sizeof(wamrHeapBuffer);

    // Returns true if success, false otherwise
    return wasm_runtime_full_init(&wamrRteArgs);
}

EnclaveWasmModule::EnclaveWasmModule() {}

EnclaveWasmModule::~EnclaveWasmModule()
{
    wasm_runtime_deinstantiate(moduleInstance);
    wasm_runtime_unload(wasmModule);
}

bool EnclaveWasmModule::loadWasm(void* wasmOpCodePtr, uint32_t wasmOpCodeSize)
{
    std::vector<uint8_t> wasmBytes((uint8_t*)wasmOpCodePtr,
                                   (uint8_t*)wasmOpCodePtr + wasmOpCodeSize);

    wasmModule = wasm_runtime_load(wasmBytes.data(),
                                   wasmBytes.size(),
                                   errorBuffer,
                                   FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE);

    if (wasmModule == nullptr) {
        return false;
    }

    moduleInstance =
      wasm_runtime_instantiate(wasmModule,
                               FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE,
                               FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE,
                               errorBuffer,
                               FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE);

    return moduleInstance != nullptr;
}

bool EnclaveWasmModule::callFunction()
{
    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, WASM_ENTRY_FUNC, nullptr);

    // Set argv to capture return value
    std::vector<uint32_t> argv = { 0 };

    bool success =
      aot_create_exec_env_and_call_function((AOTModuleInstance*)moduleInstance,
                                            (AOTFunctionInstance*)func,
                                            0x0,
                                            argv.data());

    if (success) {
        ocallLogDebug("Success calling WASM function");
    } else {
        std::string errorMessage(
          ((AOTModuleInstance*)moduleInstance)->cur_exception);
        // TODO - better logging
        std::string errorText =
          "Caught WASM runtime exception: " + errorMessage;
        ocallLogError(errorText.c_str());
    }

    return success;
}
}
