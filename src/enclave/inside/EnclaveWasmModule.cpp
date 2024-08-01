#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>
#include <wasm/WasmCommon.h>

#include <string>
#include <vector>

namespace wasm {

// Define the module map and its mutex
std::unordered_map<uint32_t, std::shared_ptr<EnclaveWasmModule>> moduleMap;
std::mutex moduleMapMutex;

static bool wamrInitialised = false;
static uint8_t wamrHeapBuffer[WAMR_HEAP_BUFFER_SIZE];

bool EnclaveWasmModule::initialiseWAMRGlobally()
{
    // We want to initialise WAMR's globals once for all modules, but we know
    // we have taken a lock outside the enclave, so we don't need to take a
    // lock again here
    if (wamrInitialised) {
        return true;
    }

    // Initialise the WAMR runtime
    RuntimeInitArgs wamrRteArgs;
    memset(&wamrRteArgs, 0x0, sizeof(wamrRteArgs));
    // wamrRteArgs.mem_alloc_type = Alloc_With_Allocator;
    wamrRteArgs.mem_alloc_type = Alloc_With_Pool;
    wamrRteArgs.mem_alloc_option.pool.heap_buf = (void*)wamrHeapBuffer;
    wamrRteArgs.mem_alloc_option.pool.heap_size = sizeof(wamrHeapBuffer);

    // Initialise WAMR runtime
    bool success = wasm_runtime_full_init(&wamrRteArgs);

    // Register native symbols
    sgx::initialiseSGXWAMRNatives();

    wamrInitialised = true;

    return success;
}

EnclaveWasmModule::EnclaveWasmModule() {}

EnclaveWasmModule::~EnclaveWasmModule()
{
    wasm_runtime_deinstantiate(moduleInstance);
    wasm_runtime_unload(wasmModule);
}

bool EnclaveWasmModule::reset(const std::string& user, const std::string& func)
{
    bool sucess = true;

    if (!_isBound) {
        return sucess;
    }

    // Sanity check
    if (boundUser != user || boundFunction != func) {
        SPDLOG_ERROR_SGX("Mismatch when resetting SGX-WAMR module!");
        SPDLOG_ERROR_SGX("Bound user/func (%s/%s) != reset user/func (%s/%s)",
                         boundUser.c_str(),
                         boundFunction.c_str(),
                         user.c_str(),
                         func.c_str());

        sucess = false;
        return sucess;
    }

    SPDLOG_DEBUG_SGX(
      "SGX-WAMR resetting after %s/%s", user.c_str(), func.c_str());
    wasm_runtime_deinstantiate(moduleInstance);
    sucess = bindInternal();

    return sucess;
}

bool EnclaveWasmModule::doBindToFunction(const std::string& user,
                                         const std::string& func,
                                         void* wasmOpCodePtr,
                                         uint32_t wasmOpCodeSize)
{
    if (_isBound) {
        SPDLOG_ERROR_SGX("EnclaveWasmModule already bound!");
        return false;
    }

    std::vector<uint8_t> wasmBytes((uint8_t*)wasmOpCodePtr,
                                   (uint8_t*)wasmOpCodePtr + wasmOpCodeSize);

    wasmModule = wasm_runtime_load(
      wasmBytes.data(), wasmBytes.size(), errorBuffer, WAMR_ERROR_BUFFER_SIZE);

    if (wasmModule == nullptr) {
        SPDLOG_ERROR_SGX(
          "Error loading WASM for %s/%s", user.c_str(), func.c_str());
        return false;
    }

    if (!bindInternal()) {
        SPDLOG_ERROR_SGX(
          "Error instantiating WASM for %s/%s", user.c_str(), func.c_str());
        return false;
    }

    _isBound = true;
    boundUser = user;
    boundFunction = func;

    return true;
}

bool EnclaveWasmModule::bindInternal()
{
    // Instantiate module. Set the app-managed heap size to 0 to use
    // wasi-libc's managed heap. See:
    // https://bytecodealliance.github.io/wamr.dev/blog/understand-the-wamr-heap/
    moduleInstance = wasm_runtime_instantiate(
      wasmModule, WAMR_STACK_SIZE, 0, errorBuffer, WAMR_ERROR_BUFFER_SIZE);

    // Sense-check the module after instantiation
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];

    if (aotMem->num_bytes_per_page != WASM_BYTES_PER_PAGE) {
        SPDLOG_ERROR_SGX(
          "WAMR module bytes per page wrong, %i != %i, overriding",
          aotMem->num_bytes_per_page,
          WASM_BYTES_PER_PAGE);
        throw std::runtime_error("WAMR module bytes per page wrong");
    }

    if (moduleInstance == nullptr) {
        std::string errorMsg = std::string(errorBuffer);
        SPDLOG_ERROR_SGX("Failed to instantiate WAMR module: \n%s",
                         errorMsg.c_str());
        throw std::runtime_error("Failed to instantiate WAMR module");
    }
    // TODO(mman)
    // currentBrk.store(getMemorySizeBytes(), std::memory_order_release);

    return moduleInstance != nullptr;
}

uint32_t EnclaveWasmModule::callFunction(uint32_t argcIn, char** argvIn)
{
    int returnValue = 0;
    // First, run wasm initialisers
    returnValue = executeWasmFunction(WASM_CTORS_FUNC_NAME);
    if (returnValue != 0) {
        SPDLOG_ERROR_SGX("Error executing WASM ctors function");
        throw std::runtime_error("Error executing WASM ctors function");
    }

    // Second, run the entrypoint in the WASM module
    prepareArgcArgv(argcIn, argvIn);
    returnValue = executeWasmFunction(ENTRY_FUNC_NAME);

    return returnValue;
}

// TODO(csegarragonz): merge this with the implementation in WAMRWasmModule
int EnclaveWasmModule::executeWasmFunction(const std::string& funcName)
{
    SPDLOG_DEBUG_SGX("WAMR executing function from string %s",
                     funcName.c_str());

    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, funcName.c_str(), nullptr);
    if (func == nullptr) {
        SPDLOG_ERROR_SGX("Did not find function %s", funcName.c_str());
        throw std::runtime_error("Did not find named wasm function");
    }

    // Note, for some reason WAMR sets the return value in the argv array you
    // pass it, therefore we should provide a single integer argv even though
    // it's not actually used
    std::vector<uint32_t> argv = { 0 };

    // Create local execution enviornment and call function
    auto* execEnv =
      wasm_runtime_create_exec_env(moduleInstance, WAMR_STACK_SIZE);
    bool success = wasm_runtime_call_wasm(execEnv, func, argc, argv.data());
    wasm_runtime_destroy_exec_env(execEnv);

    uint32_t returnValue = argv[0];

    // Check function result
    if (!success || returnValue != 0) {
        std::string errorMessage(
          ((AOTModuleInstance*)moduleInstance)->cur_exception);

        // Strip the prefix that WAMR puts on internally to exceptions
        errorMessage = errorMessage.substr(
          std::string(WAMR_INTERNAL_EXCEPTION_PREFIX).size(),
          errorMessage.size());
        SPDLOG_ERROR_SGX("Error message: %s", errorMessage.c_str());

        // If we have set the exit code in the host interface (through
        // wasi_proc_exit) then we remove the prefix to parse the return
        // value
        if (errorMessage.rfind(WAMR_EXIT_PREFIX, 0) != std::string::npos) {
            std::string returnValueString = errorMessage.substr(
              std::string(WAMR_EXIT_PREFIX).size(), errorMessage.size());
            int parsedReturnValue = std::stoi(returnValueString);

            SPDLOG_ERROR_SGX("Caught WAMR exit code %i (from %s)",
                             parsedReturnValue,
                             errorMessage.c_str());
            return parsedReturnValue;
        }

        // SPDLOG_ERROR_SGX("Caught wasm runtime exception: %s",
        // errorMessage.c_str());

        // Ensure return value is not zero if not successful
        if (returnValue == 0) {
            returnValue = 1;
        }

        return returnValue;
    }

    SPDLOG_DEBUG_SGX("WAMR finished executing %s", funcName.c_str());
    return returnValue;
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

uint32_t EnclaveWasmModule::getArgc() const
{
    return argc;
}

std::vector<std::string> EnclaveWasmModule::getArgv()
{
    return argv;
}

size_t EnclaveWasmModule::getArgvBufferSize() const
{
    return argvBufferSize;
}

// Validate that a memory range defined by a pointer and a size is a valid
// offset in the module's WASM linear memory.
void EnclaveWasmModule::validateNativePointer(void* nativePtr, int size)
{
    bool success =
      wasm_runtime_validate_native_addr(moduleInstance, nativePtr, size);

    if (!success) {
        throw std::runtime_error("Failed validating native pointer!");
    }
}

// We wrap around exception throwing in SGX enclaves in case we decide that
// there is a better way to handle errors than just throwing exceptions. It
// is likely that throwing exceptions from SGX together with WAMR's C stack
// frames is gonna really mess things up.
void EnclaveWasmModule::doThrowException(std::exception& exc) const
{
    SPDLOG_DEBUG_SGX("Throwing exception for %s/%s",
                     getBoundUser().c_str(),
                     getBoundFunction().c_str());

    throw exc;
}

std::shared_ptr<EnclaveWasmModule> getExecutingEnclaveWasmModule(
  wasm_exec_env_t execEnv)
{
    // Acquiring a lock every time may be too conservative
    std::unique_lock<std::mutex> lock(moduleMapMutex);
    for (auto& itr : moduleMap) {
        if (itr.second->getModuleInstance() == execEnv->module_inst) {
            return itr.second;
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
