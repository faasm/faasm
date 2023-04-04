#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/ocalls.h>

#include <string>
#include <vector>

namespace wasm {

// Define the module map and its mutex
std::unordered_map<uint32_t, std::shared_ptr<EnclaveWasmModule>> moduleMap;
std::mutex moduleMapMutex;

// Define the WAMR's heap buffer
static uint8_t wamrHeapBuffer[FAASM_SGX_WAMR_HEAP_SIZE];

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

bool EnclaveWasmModule::callFunction(uint32_t argcIn, char** argvIn)
{
    prepareArgcArgv(argcIn, argvIn);

    WASMExecEnv* execEnv = wasm_runtime_get_exec_env_singleton(moduleInstance);
    if (execEnv == nullptr) {
        ocallLogError("Failed to create WAMR exec env");
        throw std::runtime_error("Failed to create WAMR exec env");
    }

    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, WASM_ENTRY_FUNC, nullptr);
    if (func == nullptr) {
        ocallLogError("Did not find named WASM function");
        throw std::runtime_error("Did not find named wasm function");
    }

    // Set dummy argv to capture return value
    std::vector<uint32_t> argv = { 0 };
    bool success = wasm_runtime_call_wasm(execEnv, func, 0, argv.data());
    uint32_t returnValue = argv[0];

    if (success) {
        ocallLogDebug("Success calling WASM function");
    } else {
        std::string errorMessage(wasm_runtime_get_exception(moduleInstance));
        std::string errorText =
          "Caught WASM runtime exception: " + errorMessage;
        ocallLogError(errorText.c_str());
    }

    return success;
}

WASMModuleInstanceCommon* EnclaveWasmModule::getModuleInstance()
{
    return moduleInstance;
}

void EnclaveWasmModule::prepareArgcArgv(uint32_t argcIn, char** argvIn)
{
    argc = argcIn;
    argv.resize(argc);
    argvBufferSize = 0;

    for (int i = 0; i < argc; i++) {
        // Add one to account for newline
        argvBufferSize += strlen(argvIn[i]) + 1;
        argv.at(i) = std::string(argvIn[i]);
    }
}

uint32_t EnclaveWasmModule::getArgc()
{
    return argc;
}

std::vector<std::string> EnclaveWasmModule::getArgv()
{
    return argv;
}

size_t EnclaveWasmModule::getArgvBufferSize()
{
    return argvBufferSize;
}

std::shared_ptr<EnclaveWasmModule> getExecutingEnclaveWasmModule(
  wasm_exec_env_t execEnv)
{
    // Acquiring a lock every time may be too conservative
    std::unique_lock<std::mutex> lock(moduleMapMutex);
    for (auto& it : moduleMap) {
        if (it.second->getModuleInstance() == execEnv->module_inst) {
            return it.second;
        }
    }

    // Returning a null ptr means that we haven't been able to link the
    // execution environment to any of the registered modules. This is a fatal
    // error, but we expect the caller to handle it, as throwing exceptions
    // is not supported.
    ocallLogError("Can not find any registered module corresponding to the "
                  "supplied execution environment, this is a fatal error");

    return nullptr;
}

}
