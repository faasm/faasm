#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>
#include <wasm/WasmCommon.h>

#include <string>

namespace wasm {

static bool wamrInitialised = false;

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
    wamrRteArgs.mem_alloc_type = Alloc_With_Allocator;
    wamrRteArgs.mem_alloc_option.allocator.malloc_func = (void*)::malloc;
    wamrRteArgs.mem_alloc_option.allocator.realloc_func = (void*)::realloc;
    wamrRteArgs.mem_alloc_option.allocator.free_func = (void*)::free;

    // Initialise WAMR runtime
    bool success = wasm_runtime_full_init(&wamrRteArgs);

    // Register native symbols
    sgx::initialiseSGXWAMRNatives();

    wamrInitialised = true;

    // Set log level: BH_LOG_LEVEL_{FATAL,ERROR,WARNING,DEBUG,VERBOSE}
    bh_log_set_verbose_level(BH_LOG_LEVEL_WARNING);

    return success;
}

EnclaveWasmModule::EnclaveWasmModule()
{
    // Initialize enclave's public/private key
    sgx_status_t ret = sgx_ecc256_open_context(&this->keyContext);
    if (ret != SGX_SUCCESS) {
        SPDLOG_ERROR_SGX("Error intializing crypto context for enclave!");
    }

    // Generate public/private key pair
    ret = sgx_ecc256_create_key_pair(
      &this->privateKey, &this->publicKey, this->keyContext);
}

EnclaveWasmModule::~EnclaveWasmModule()
{
    if (this->keyContext != nullptr) {
        sgx_ecc256_close_context(this->keyContext);
    }

    wasm_runtime_deinstantiate(moduleInstance);
    wasm_runtime_unload(wasmModule);
    // Free the module bytes
    if (wasmModuleBytes != nullptr) {
        free(wasmModuleBytes);
    }
    wasmModuleBytes = nullptr;
    wasmModuleBytesSize = 0;
}

bool EnclaveWasmModule::reset()
{
    bool sucess = true;

    if (!_isBound) {
        return sucess;
    }

    SPDLOG_DEBUG_SGX(
      "SGX-WAMR resetting after %s/%s", user.c_str(), function.c_str());
    wasm_runtime_deinstantiate(moduleInstance);
    sucess = bindInternal();

    return sucess;
}

bool EnclaveWasmModule::doBindToFunction(const char* user,
                                         const char* function,
                                         uint8_t* wasmBytesPtr,
                                         uint32_t wasmBytesSize)
{
    if (_isBound) {
        SPDLOG_ERROR_SGX("EnclaveWasmModule already bound!");
        return false;
    }

    this->user = std::string(user);
    this->function = std::string(function);

    // According to the docs, the byte array we give to wasm_runtime_load
    // must be usable until we call wasm_runtime_unload
    this->wasmModuleBytes = (uint8_t*)malloc(wasmBytesSize);
    this->wasmModuleBytesSize = wasmBytesSize;
    memcpy(this->wasmModuleBytes, wasmBytesPtr, this->wasmModuleBytesSize);

    wasmModule = wasm_runtime_load(this->wasmModuleBytes,
                                   this->wasmModuleBytesSize,
                                   errorBuffer,
                                   WAMR_ERROR_BUFFER_SIZE);

    if (wasmModule == nullptr) {
        SPDLOG_ERROR_SGX(
          "Error loading WASM for %s/%s: %s", user, function, errorBuffer);
        return false;
    }

    if (!bindInternal()) {
        SPDLOG_ERROR_SGX("Error instantiating WASM for %s/%s", user, function);
        return false;
    }

    _isBound = true;

    return true;
}

bool EnclaveWasmModule::bindInternal()
{
    // Instantiate module. Set the app-managed heap size to 0 to use
    // wasi-libc's managed heap. See:
    // https://bytecodealliance.github.io/wamr.dev/blog/understand-the-wamr-heap/
    moduleInstance = wasm_runtime_instantiate(
      wasmModule, WAMR_STACK_SIZE, 0, errorBuffer, WAMR_ERROR_BUFFER_SIZE);

    if (moduleInstance == nullptr) {
        SPDLOG_ERROR_SGX("Null-pointing module instance for %s/%s: %s",
                         user.c_str(),
                         function.c_str(),
                         errorBuffer);
        return false;
    }

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

    currentBrk = getMemorySizeBytes();

    return moduleInstance != nullptr;
}

uint32_t EnclaveWasmModule::callFunction(uint32_t argcIn, char** argvIn)
{
    int returnValue = 0;

    // Second, run the entrypoint in the WASM module
    prepareArgcArgv(argcIn, argvIn);
    returnValue = executeWasmFunction(ENTRY_FUNC_NAME);

    // When running the main function (_start in WASI) we want to overwrite
    // the function's return value for the one in WAMR's WASI context.
    // The former is just the return value of _start, whereas the latter
    // is the actual return value of the entrypoint (e.g. main)
    returnValue = wasm_runtime_get_wasi_ctx(moduleInstance)->exit_code;

    return returnValue;
}

// TODO(csegarragonz): merge this with the implementation in WAMRWasmModule
int EnclaveWasmModule::executeWasmFunction(const std::string& funcName)
{
    SPDLOG_DEBUG_SGX("WAMR executing function from string %s",
                     funcName.c_str());

    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, funcName.c_str());
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

    for (uint32_t i = 0; i < argc; i++) {
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

uint32_t EnclaveWasmModule::getCurrentBrk() const
{
    return currentBrk;
}

uint32_t EnclaveWasmModule::shrinkMemory(size_t nBytes)
{
    if (!isWasmPageAligned((int32_t)nBytes)) {
        SPDLOG_ERROR_SGX("Shrink size not page aligned %li", nBytes);
        throw std::runtime_error("New break not page aligned");
    }

    uint32_t oldBrk = currentBrk;

    if (nBytes > oldBrk) {
        SPDLOG_ERROR_SGX(
          "Shrinking by more than current brk (%li > %i)", nBytes, oldBrk);
        throw std::runtime_error("Shrinking by more than current brk");
    }

    // Note - we don't actually free the memory, we just change the brk
    uint32_t newBrk = oldBrk - nBytes;

    SPDLOG_DEBUG_SGX("MEM - shrinking memory %i -> %i", oldBrk, newBrk);
    currentBrk = newBrk;

    return oldBrk;
}

uint32_t EnclaveWasmModule::growMemory(size_t nBytes)
{
    if (nBytes == 0) {
        return currentBrk;
    }

    uint32_t oldBytes = getMemorySizeBytes();
    uint32_t oldBrk = currentBrk;
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
        SPDLOG_DEBUG_SGX(
          "MEM - Growing memory using already provisioned %u + %li <= %i",
          oldBrk,
          nBytes,
          oldBytes);

        currentBrk = newBrk;

        // TODO: we don't claim the virtual memory, permissions on the memory
        // pages could be off

        return oldBrk;
    }

    uint32_t pageChange = newPages - oldPages;
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    bool success = aot_enlarge_memory(aotModule, pageChange);
    if (!success) {
        SPDLOG_ERROR_SGX("Failed to grow memory (page change: %i): %s",
                         pageChange,
                         wasm_runtime_get_exception(moduleInstance));
        throw std::runtime_error("Failed to grow memory");
    }

    SPDLOG_DEBUG_SGX("Growing memory from %i to %i pages (max %li)",
                     oldPages,
                     newPages,
                     maxPages);

    size_t newMemorySize = getMemorySizeBytes();
    currentBrk = newMemorySize;

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
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];
    return aotMem->cur_page_count * WASM_BYTES_PER_PAGE;
}

uint8_t* EnclaveWasmModule::getMemoryBase()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];
    return reinterpret_cast<uint8_t*>(aotMem->memory_data);
}

size_t EnclaveWasmModule::getMaxMemoryPages()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];
    return aotMem->max_page_count;
}

uint32_t EnclaveWasmModule::mmapMemory(size_t nBytes)
{
    // The mmap interface allows non page-aligned values, and rounds up
    uint32_t pageAligned = roundUpToWasmPageAligned(nBytes);
    return growMemory(pageAligned);
}

uint32_t EnclaveWasmModule::mmapFile(uint32_t wasmFd, size_t length)
{
    SPDLOG_ERROR_SGX("mmap for files is not implemented!");

    throw std::runtime_error("mmap for files not implemented!");
}

void EnclaveWasmModule::unmapMemory(uint32_t offset, size_t nBytes)
{
    // TODO: remove duplication with WasmModule
    if (nBytes == 0) {
        return;
    }

    // Munmap expects the offset itself to be page-aligned, but will round up
    // the number of bytes
    if (!isWasmPageAligned(offset)) {
        SPDLOG_ERROR_SGX("Non-page aligned munmap address %i", offset);
        throw std::runtime_error("Non-page aligned munmap address");
    }

    uint32_t pageAligned = roundUpToWasmPageAligned(nBytes);
    size_t maxPages = getMaxMemoryPages();
    size_t maxSize = maxPages * WASM_BYTES_PER_PAGE;
    uint32_t unmapTop = offset + pageAligned;

    if (unmapTop > maxSize) {
        SPDLOG_ERROR_SGX(
          "Munmapping outside memory max (%i > %zu)", unmapTop, maxSize);
        throw std::runtime_error("munmapping outside memory max");
    }

    if (unmapTop == currentBrk) {
        shrinkMemory(pageAligned);
    } else {
        SPDLOG_ERROR_SGX("MEM - unable to reclaim unmapped memory %i at %i",
                         pageAligned,
                         offset);
    }
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

EnclaveWasmModule* getExecutingEnclaveWasmModule()
{
    static EnclaveWasmModule enclaveWasmModule;
    return &enclaveWasmModule;
}

EnclaveWasmModule* getExecutingEnclaveWasmModule(wasm_exec_env_t execEnv)
{
    auto* enclaveWasmModule = getExecutingEnclaveWasmModule();

    // Sanity-check in debug mode
#ifdef FAASM_SGX_DEBUG
    if (enclaveWasmModule->getModuleInstance() != execEnv->module_inst) {
        ocallLogError(
          "Enclave WASM module bound to different module instance!");
        return nullptr;
    }
#endif

    return enclaveWasmModule;
}
}
