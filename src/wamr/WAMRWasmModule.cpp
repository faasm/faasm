#include <storage/FileLoader.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>

#include <cstdint>
#include <stdexcept>
#include <stdlib.h>
#include <sys/mman.h>

#include <aot_runtime.h>
#include <platform_common.h>
#include <wasm_exec_env.h>
#include <wasm_export.h>

namespace wasm {
// The high level API for WAMR can be found here:
// https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/core/iwasm/include/wasm_export.h
static bool wamrInitialised = false;

// WAMR maintains some global state, which we must be careful not to modify
// concurrently from our side. We are deliberately cautious with this locking,
// so it may cause performance issues under high churn of short-lived functions.
static std::mutex wamrGlobalsMutex;

void WAMRWasmModule::initialiseWAMRGlobally()
{
    faabric::util::UniqueLock lock(wamrGlobalsMutex);

    if (wamrInitialised) {
        return;
    }

    // Initialise WAMR runtime
    bool success = wasm_runtime_init();
    if (!success) {
        throw std::runtime_error("Failed to initialise WAMR");
    }

    SPDLOG_DEBUG("Successfully initialised WAMR");

    // Initialise Faasm's own native symbols
    initialiseWAMRNatives();
}

WAMRWasmModule::WAMRWasmModule()
{
    initialiseWAMRGlobally();
}

WAMRWasmModule::WAMRWasmModule(int threadPoolSizeIn)
  : WasmModule(threadPoolSizeIn)
{
    initialiseWAMRGlobally();
}

WAMRWasmModule::~WAMRWasmModule()
{
    SPDLOG_TRACE(
      "Destructing WAMR wasm module {}/{}", boundUser, boundFunction);

    faabric::util::UniqueLock lock(wamrGlobalsMutex);

    wasm_runtime_deinstantiate(moduleInstance);
    wasm_runtime_unload(wasmModule);
}

WAMRWasmModule* getExecutingWAMRModule()
{
    return reinterpret_cast<WAMRWasmModule*>(getExecutingModule());
}

// ----- Module lifecycle -----

void WAMRWasmModule::reset(faabric::Message& msg,
                           const std::string& snapshotKey)
{
    if (!_isBound) {
        return;
    }

    std::string funcStr = faabric::util::funcToString(msg, true);
    SPDLOG_DEBUG("WAMR resetting after {} (snap key {})", funcStr, snapshotKey);

    bindInternal(msg);
}

void WAMRWasmModule::doBindToFunction(faabric::Message& msg, bool cache)
{
    SPDLOG_TRACE("WAMR binding to {}/{} via message {}",
                 msg.user(),
                 msg.function(),
                 msg.id());

    // Prepare the filesystem
    filesystem.prepareFilesystem();

    // Load the wasm file
    storage::FileLoader& functionLoader = storage::getFileLoader();
    wasmBytes = functionLoader.loadFunctionWamrAotFile(msg);

    {
        faabric::util::UniqueLock lock(wamrGlobalsMutex);
        SPDLOG_TRACE("WAMR loading {} wasm bytes\n", wasmBytes.size());
        wasmModule = wasm_runtime_load(
          wasmBytes.data(), wasmBytes.size(), errorBuffer, ERROR_BUFFER_SIZE);

        if (wasmModule == nullptr) {
            std::string errorMsg = std::string(errorBuffer);
            SPDLOG_ERROR("Failed to load WAMR module: \n{}", errorMsg);
            throw std::runtime_error("Failed to load WAMR module");
        }
    }

    bindInternal(msg);
}

void WAMRWasmModule::bindInternal(faabric::Message& msg)
{
    // Instantiate module
    moduleInstance = wasm_runtime_instantiate(
      wasmModule, STACK_SIZE_KB, HEAP_SIZE_KB, errorBuffer, ERROR_BUFFER_SIZE);

    // Sense-check the module
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];

    if (aotMem->num_bytes_per_page != WASM_BYTES_PER_PAGE) {
        SPDLOG_ERROR("WAMR module bytes per page wrong, {} != {}, overriding",
                     aotMem->num_bytes_per_page,
                     WASM_BYTES_PER_PAGE);
        throw std::runtime_error("WAMR module bytes per page wrong");
    }

    if (moduleInstance == nullptr) {
        std::string errorMsg = std::string(errorBuffer);
        SPDLOG_ERROR("Failed to instantiate WAMR module: \n{}", errorMsg);
        throw std::runtime_error("Failed to instantiate WAMR module");
    }
    currentBrk.store(getMemorySizeBytes(), std::memory_order_release);
    // Set up thread stacks
    createThreadStacks();
}

int32_t WAMRWasmModule::executeFunction(faabric::Message& msg)
{
    SPDLOG_DEBUG("WAMR executing message {}", msg.id());

    // Make sure context is set
    WasmExecutionContext ctx(this);
    int returnValue = 0;

    // Run wasm initialisers
    executeWasmFunction(WASM_CTORS_FUNC_NAME);

    if (msg.funcptr() > 0) {
        // Run the function from the pointer
        returnValue = executeWasmFunctionFromPointer(msg.funcptr());
    } else {
        prepareArgcArgv(msg);

        // Run the main function
        returnValue = executeWasmFunction(ENTRY_FUNC_NAME);
    }

    // Record the return value
    msg.set_returnvalue(returnValue);

    return returnValue;
}

int WAMRWasmModule::executeWasmFunctionFromPointer(int wasmFuncPtr)
{
    // NOTE: WAMR doesn't provide a nice interface for calling functions using
    // function pointers, so we have to call a few more low-level functions to
    // get it to work.

    std::unique_ptr<WASMExecEnv, decltype(&wasm_exec_env_destroy)> execEnv(
      wasm_exec_env_create(moduleInstance, STACK_SIZE_KB),
      &wasm_exec_env_destroy);
    if (execEnv == nullptr) {
        SPDLOG_ERROR("Failed to create exec env for func ptr {}", wasmFuncPtr);
        throw std::runtime_error("Failed to create WAMR exec env");
    }

    // Set thread handle and stack boundary (required by WAMR)
    wasm_exec_env_set_thread_info(execEnv.get());

    // Call the function pointer
    // NOTE: for some reason WAMR uses the argv array to pass the function
    // return value, so we have to provide something big enough
    std::vector<uint32_t> argv = { 0 };
    bool success =
      wasm_runtime_call_indirect(execEnv.get(), wasmFuncPtr, 0, argv.data());

    uint32_t returnValue = argv[0];

    // Handle errors
    if (!success || returnValue != 0) {
        std::string errorMessage(
          ((AOTModuleInstance*)moduleInstance)->cur_exception);

        SPDLOG_ERROR("Failed to execute from function pointer {}: {}",
                     wasmFuncPtr,
                     returnValue);

        return returnValue;
    }

    return returnValue;
}

int WAMRWasmModule::executeWasmFunction(const std::string& funcName)
{
    SPDLOG_TRACE("WAMR executing function from string {}", funcName);

    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, funcName.c_str(), nullptr);
    if (func == nullptr) {
        SPDLOG_ERROR("Did not find function {} for module {}/{}",
                     funcName,
                     boundUser,
                     boundFunction);
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

        // Strip the prefix that WAMR puts on internally
        errorMessage = faabric::util::removeSubstr(
          errorMessage, WAMR_INTERNAL_EXCEPTION_PREFIX);

        // Special case where we've set the exit code from within the host
        // interface
        if (faabric::util::startsWith(errorMessage, WAMR_EXIT_PREFIX)) {
            std::string returnValueString =
              faabric::util::removeSubstr(errorMessage, WAMR_EXIT_PREFIX);
            int parsedReturnValue = std::stoi(returnValueString);

            SPDLOG_ERROR("Caught WAMR exit code {} (from {})",
                         parsedReturnValue,
                         errorMessage);
            return parsedReturnValue;
        }

        SPDLOG_ERROR("Caught wasm runtime exception: {}", errorMessage);

        // Ensure return value is not zero if not successful
        if (returnValue == 0) {
            returnValue = 1;
        }

        return returnValue;
    }

    SPDLOG_DEBUG("WAMR finished executing {}", funcName);
    return returnValue;
}

void WAMRWasmModule::writeStringToWasmMemory(const std::string& strHost,
                                             char* strWasm)
{
    validateNativePointer(strWasm, strHost.size());
    std::copy(strHost.begin(), strHost.end(), strWasm);
}

void WAMRWasmModule::writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm,
                                              char* envBuffWasm)
{
    writeStringArrayToMemory(
      wasmEnvironment.getVars(), envOffsetsWasm, envBuffWasm);
}

void WAMRWasmModule::validateWasmOffset(uint32_t wasmOffset, size_t size)
{
    if (!wasm_runtime_validate_app_addr(moduleInstance, wasmOffset, size)) {
        SPDLOG_ERROR("WASM offset outside WAMR module instance memory"
                     "(offset: {}, size: {})",
                     wasmOffset,
                     size);
        throw std::runtime_error("Offset outside WAMR's memory");
    }
}

uint8_t* WAMRWasmModule::wasmPointerToNative(uint32_t wasmPtr)
{
    void* nativePtr = wasm_runtime_addr_app_to_native(moduleInstance, wasmPtr);
    if (nativePtr == nullptr) {
        SPDLOG_ERROR("WASM offset {} is out of the WAMR module's address space",
                     wasmPtr);
        throw std::runtime_error("Offset out of WAMR memory");
    }
    return static_cast<uint8_t*>(nativePtr);
}

uint32_t WAMRWasmModule::growMemory(size_t nBytes)
{

    uint32_t oldBytes = getMemorySizeBytes();
    uint32_t oldBrk = currentBrk.load(std::memory_order_acquire);
    uint32_t newBrk = oldBrk + nBytes;

    if (!isWasmPageAligned(newBrk)) {
        SPDLOG_ERROR("Growing WAMR memory by {} is not wasm page aligned"
                     " (current brk: {}, new brk: {})",
                     nBytes,
                     oldBrk,
                     newBrk);
        throw std::runtime_error("Non-wasm-page-aligned WAMR memory growth");
    }

    size_t newBytes = oldBytes + nBytes;
    uint32_t oldPages = getNumberOfWasmPagesForBytes(oldBytes);
    uint32_t newPages = getNumberOfWasmPagesForBytes(newBytes);
    size_t maxPages = getMaxMemoryPages();

    if (newPages > maxPages) {
        SPDLOG_ERROR(
          "WAMR grow memory would exceed max of {} pages (requested {})",
          maxPages,
          newPages);
        throw std::runtime_error("WAMR grow memory exceeding max");
    }

    uint32_t pageChange = newPages - oldPages;
    bool success = wasm_runtime_enlarge_memory(moduleInstance, pageChange);
    if (!success) {
        throw std::runtime_error("Failed to grow WAMR memory");
    }

    SPDLOG_TRACE("Growing WAMR memory from {} to {} pages", oldPages, newPages);

    size_t newMemorySize = getMemorySizeBytes();
    currentBrk.store(newMemorySize, std::memory_order_release);
    if (newMemorySize != newBytes) {
        SPDLOG_ERROR(
          "Expected new brk ({}) to be old WAMR memory plus new bytes ({})",
          newMemorySize,
          newBytes);
        throw std::runtime_error("WAMR memory growth discrepancy");
    }

    return oldBrk;
}

uint32_t WAMRWasmModule::shrinkMemory(size_t nBytes)
{

    SPDLOG_WARN("WAMR ignoring shrink memory");
    return 0;
}

uint32_t WAMRWasmModule::mmapMemory(size_t nBytes)
{
    return growMemory(nBytes);
}

size_t WAMRWasmModule::getMemorySizeBytes()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return aotMem->cur_page_count * WASM_BYTES_PER_PAGE;
}

uint8_t* WAMRWasmModule::getMemoryBase()
{
    auto aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return reinterpret_cast<uint8_t*>(aotMem->memory_data.ptr);
}

size_t WAMRWasmModule::getMaxMemoryPages()
{
    auto aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return aotMem->max_page_count;
}

WASMModuleInstanceCommon* WAMRWasmModule::getModuleInstance()
{
    if (moduleInstance == nullptr) {
        SPDLOG_ERROR("Trying to get a null-pointing module instance");
        throw std::runtime_error("Null WAMR module instance");
    }
    return moduleInstance;
}

std::vector<std::string> WAMRWasmModule::getArgv()
{
    return argv;
}

uint32_t WAMRWasmModule::mmapFile(uint32_t fp, size_t length)
{
    // TODO - implement
    return 0;
}
}
