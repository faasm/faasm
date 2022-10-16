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

// Define the WAMR's heap buffer
static uint8_t wamrHeapBuffer[WAMR_HEAP_BUFFER_SIZE];

bool EnclaveWasmModule::initialiseWAMRGlobally()
{
    // We want to initialise WAMR's globals once for all modules, but we know
    // we have taken a lock outside the enclave, so we don't need to take a
    // lock again here
    if (wamrInitialised) {
        return true;
    }

    // Set WAMR log level
    bh_log_set_verbose_level(4);

    // Initialise the WAMR runtime
    RuntimeInitArgs wamrRteArgs;
    memset(&wamrRteArgs, 0x0, sizeof(wamrRteArgs));
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

bool EnclaveWasmModule::loadWasm(void* wasmOpCodePtr, uint32_t wasmOpCodeSize)
{
    std::vector<uint8_t> wasmBytes((uint8_t*)wasmOpCodePtr,
                                   (uint8_t*)wasmOpCodePtr + wasmOpCodeSize);

    wasmModule = wasm_runtime_load(
      wasmBytes.data(), wasmBytes.size(), errorBuffer, WAMR_ERROR_BUFFER_SIZE);

    if (wasmModule == nullptr) {
        return false;
    }

    moduleInstance = wasm_runtime_instantiate(wasmModule,
                                              WAMR_STACK_SIZE,
                                              WAMR_HEAP_SIZE,
                                              errorBuffer,
                                              WAMR_ERROR_BUFFER_SIZE);

    // Sense-check the module after instantiation
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];

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
    currentBrk.store(getMemorySizeBytes(), std::memory_order_release);

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

    // Invoke the function
    bool success = aot_create_exec_env_and_call_function(
      reinterpret_cast<AOTModuleInstance*>(moduleInstance),
      reinterpret_cast<AOTFunctionInstance*>(func),
      0,
      argv.data());

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
    SPDLOG_ERROR_SGX("Can not find any registered module corresponding to the "
                     "supplied execution environment, this is a fatal error");

    return nullptr;
}

uint32_t EnclaveWasmModule::getCurrentBrk() const
{
    return currentBrk.load(std::memory_order_acquire);
}

uint32_t EnclaveWasmModule::shrinkMemory(size_t nBytes)
{
    if (!isWasmPageAligned((int32_t)nBytes)) {
        SPDLOG_ERROR_SGX("Shrink size not page aligned %li", nBytes);
        throw std::runtime_error("New break not page aligned");
    }

    std::unique_lock<std::mutex> lock(moduleMutex);

    uint32_t oldBrk = currentBrk.load(std::memory_order_acquire);

    if (nBytes > oldBrk) {
        SPDLOG_ERROR_SGX(
          "Shrinking by more than current brk (%li > %i)", nBytes, oldBrk);
        throw std::runtime_error("Shrinking by more than current brk");
    }

    // Note - we don't actually free the memory, we just change the brk
    uint32_t newBrk = oldBrk - nBytes;

    SPDLOG_DEBUG_SGX("MEM - shrinking memory %i -> %i", oldBrk, newBrk);
    currentBrk.store(newBrk, std::memory_order_release);

    return oldBrk;
}

uint32_t EnclaveWasmModule::growMemory(size_t nBytes)
{
    if (nBytes == 0) {
        return currentBrk.load(std::memory_order_release);
    }

    std::unique_lock<std::mutex> lock(moduleMutex);

    uint32_t oldBytes = getMemorySizeBytes();
    uint32_t oldBrk = currentBrk.load(std::memory_order_acquire);
    uint32_t newBrk = oldBrk + nBytes;

    if (!isWasmPageAligned(newBrk)) {
        SPDLOG_ERROR_SGX("Growing memory by %li is not wasm page aligned"
                         " (current brk: %i, new brk: %i)",
                         nBytes,
                         oldBrk,
                         newBrk);
        throw std::runtime_error("Non-wasm-page-aligned memory growth");
    }

    size_t newBytes = oldBytes + nBytes;
    uint32_t oldPages = getNumberOfWasmPagesForBytes(oldBytes);
    uint32_t newPages = getNumberOfWasmPagesForBytes(newBytes);
    size_t maxPages = getMaxMemoryPages();

    if (newBytes > UINT32_MAX || newPages > maxPages) {
        SPDLOG_ERROR_SGX("Growing memory would exceed max of %li pages "
                         "(current %i, requested %i)",
                         maxPages,
                         oldPages,
                         newPages);
        throw std::runtime_error("Memory growth exceeding max");
    }

    // If we can reclaim old memory, just bump the break
    if (newBrk <= oldBytes) {
        SPDLOG_ERROR_SGX("We don't reclaim memory inside SGX");
    }

    uint32_t pageChange = newPages - oldPages;
    bool success = wasm_runtime_enlarge_memory(moduleInstance, pageChange);
    // bool success = true;
    if (!success) {
        SPDLOG_ERROR_SGX("Failed to grow memory (page change: %i)", pageChange);
        throw std::runtime_error("Failed to grow memory");
    }

    SPDLOG_DEBUG_SGX("Growing memory from %i to %i pages (max %li)",
                     oldPages,
                     newPages,
                     maxPages);

    size_t newMemorySize = getMemorySizeBytes();
    currentBrk.store(newMemorySize, std::memory_order_release);

    if (newMemorySize != newBytes) {
        SPDLOG_ERROR_SGX(
          "Expected new brk (%li) to be old memory plus new bytes (%li)",
          newMemorySize,
          newBytes);
        throw std::runtime_error("Memory growth discrepancy");
    }

    return oldBrk;
}

size_t EnclaveWasmModule::getMemorySizeBytes()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return aotMem->cur_page_count * WASM_BYTES_PER_PAGE;
}

uint8_t* EnclaveWasmModule::getMemoryBase()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return reinterpret_cast<uint8_t*>(aotMem->memory_data.ptr);
}

size_t EnclaveWasmModule::getMaxMemoryPages()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return aotMem->max_page_count;
}
}
