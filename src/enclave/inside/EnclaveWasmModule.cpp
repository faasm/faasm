#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/ocalls.h>

#include <string>
#include <vector>

namespace wasm {

// Define the module map and its mutex
std::unordered_map<uint32_t, std::shared_ptr<wasm::EnclaveWasmModule>>
  moduleMap;
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
    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, WASM_ENTRY_FUNC, nullptr);

    prepareArgcArgv(argcIn, argvIn);

    // Set dummy argv to capture return value
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

bool EnclaveWasmModule::validateNativePointer(void* nativePtr, size_t size)
{
    return wasm_runtime_validate_native_addr(moduleInstance, nativePtr, size);
}

uint32_t EnclaveWasmModule::nativePointerToWasmOffset(void* nativePtr)
{
    return wasm_runtime_addr_native_to_app(moduleInstance, nativePtr);
}

std::shared_ptr<EnclaveWasmModule> getExecutingEnclaveWasmModule(
  wasm_exec_env_t execEnv)
{
    // Acquiring a lock every time may be too conservative
    std::unique_lock<std::mutex> lock(wasm::moduleMapMutex);
    for (auto& it : wasm::moduleMap) {
        if (it.second->moduleInstance == execEnv->module_inst) {
            return it.second;
        }
    }

    // Returning a null ptr means that we haven't been able to link the
    // execution environment to any of the registered modules. This is a fatal
    // error, but we expect the caller to handle it, as throwing exceptions
    // is not supported.
    return nullptr;
}

void EnclaveWasmModule::writeStringArrayToMemory(
  const std::vector<std::string>& strings,
  uint32_t* strOffsets,
  char* strBuffer)
{
    // Validate that the offset array has enough capacity to hold all offsets
    // (one per string)
    validateNativePointer(strOffsets, strings.size() * sizeof(uint32_t));

    char* nextBuffer = strBuffer;
    for (size_t i = 0; i < strings.size(); i++) {
        const std::string& thisStr = strings.at(i);

        // Validate that the WASM offset we are going to write to is within
        // the bounds of the linear memory
        validateNativePointer(nextBuffer, thisStr.size() + 1);

        std::copy(thisStr.begin(), thisStr.end(), nextBuffer);
        strOffsets[i] = nativePointerToWasmOffset(nextBuffer);

        nextBuffer += thisStr.size() + 1;
    }
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

size_t EnclaveWasmModule::getArgvBufferSize()
{
    return argvBufferSize;
}

void EnclaveWasmModule::writeArgvToWamrMemory(uint32_t* argvOffsetsWasm,
                                              char* argvBuffWasm)
{
    writeStringArrayToMemory(argv, argvOffsetsWasm, argvBuffWasm);
}
}
