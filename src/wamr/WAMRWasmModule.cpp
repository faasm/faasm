#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <storage/FileLoader.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

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
std::mutex wamrInitMx;

void WAMRWasmModule::initialiseWAMRGlobally()
{
    if (wamrInitialised) {
        return;
    } else {
        faabric::util::UniqueLock lock(wamrInitMx);

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
}

void tearDownWAMRGlobally()
{
    wasm_runtime_destroy();
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
    wasm_runtime_deinstantiate(moduleInstance);
    wasm_runtime_unload(wasmModule);
}

WAMRWasmModule* getExecutingWAMRModule()
{
    return reinterpret_cast<WAMRWasmModule*>(getExecutingModule());
}

// ----- Module lifecycle -----
void WAMRWasmModule::doBindToFunction(faabric::Message& msg, bool cache)
{
    // Prepare the filesystem
    filesystem.prepareFilesystem();

    // Load the wasm file
    storage::FileLoader& functionLoader = storage::getFileLoader();
    std::vector<uint8_t> wasmBytes =
      functionLoader.loadFunctionWamrAotFile(msg);

    // Load wasm module
    wasmModule = wasm_runtime_load(
      wasmBytes.data(), wasmBytes.size(), errorBuffer, ERROR_BUFFER_SIZE);

    if (wasmModule == nullptr) {
        std::string errorMsg = std::string(errorBuffer);
        SPDLOG_ERROR("Failed to load WAMR module: \n{}", errorMsg);
        throw std::runtime_error("Failed to load WAMR module");
    }

    // Instantiate module
    moduleInstance = wasm_runtime_instantiate(
      wasmModule, STACK_SIZE_KB, HEAP_SIZE_KB, errorBuffer, ERROR_BUFFER_SIZE);

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
    WasmExecutionContext ctx(this, &msg);
    int returnValue = 0;

    // Run wasm initialisers
    executeWasmFunction(WASM_CTORS_FUNC_NAME);

    if (msg.funcptr() > 0) {
        // Run the function from the pointer
        executeWasmFunctionFromPointer(msg.funcptr());
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

    // Handle errors
    if (!success) {
        std::string errorMessage(
          ((AOTModuleInstance*)moduleInstance)->cur_exception);

        SPDLOG_ERROR("Failed to execute from function pointer {}", wasmFuncPtr);

        return 1;
    }

    return 0;
}

int WAMRWasmModule::executeWasmFunction(const std::string& funcName)
{

    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, funcName.c_str(), nullptr);

    // Note, for some reason WAMR sets the return value in the argv array you
    // pass it, therefore we should provide a single integer argv even though
    // it's not actually used
    std::vector<uint32_t> argv = { 0 };

    // Invoke the function
    bool success = aot_create_exec_env_and_call_function(
      reinterpret_cast<AOTModuleInstance*>(moduleInstance),
      reinterpret_cast<AOTFunctionInstance*>(func),
      0x0,
      argv.data());

    // Check function result
    if (success) {
        SPDLOG_DEBUG("{} finished", funcName);
    } else {
        std::string errorMessage(
          ((AOTModuleInstance*)moduleInstance)->cur_exception);
        SPDLOG_ERROR("Caught wasm runtime exception: {}", errorMessage);

        return 1;
    }

    return 0;
}

void WAMRWasmModule::writeStringToWasmMemory(const std::string& strHost,
                                             char* strWasm)
{
    validateNativeAddress(strWasm, strHost.size());
    std::copy(strHost.begin(), strHost.end(), strWasm);
}

void WAMRWasmModule::writeStringArrayToMemory(
  const std::vector<std::string>& strings,
  uint32_t* strOffsets,
  char* strBuffer)
{
    validateNativeAddress(strOffsets, strings.size() * sizeof(uint32_t));

    char* nextBuffer = strBuffer;
    for (size_t i = 0; i < strings.size(); ++i) {
        const std::string& thisStr = strings.at(i);

        validateNativeAddress(nextBuffer, thisStr.size() + 1);

        std::copy(thisStr.begin(), thisStr.end(), nextBuffer);
        strOffsets[i] = nativePointerToWasm(nextBuffer);

        nextBuffer += thisStr.size() + 1;
    }
}

void WAMRWasmModule::writeArgvToWamrMemory(uint32_t* argvOffsetsWasm,
                                           char* argvBuffWasm)
{
    writeStringArrayToMemory(argv, argvOffsetsWasm, argvBuffWasm);
}

void WAMRWasmModule::writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm,
                                              char* envBuffWasm)
{
    writeStringArrayToMemory(
      wasmEnvironment.getVars(), envOffsetsWasm, envBuffWasm);
}

void WAMRWasmModule::validateNativeAddress(void* nativePtr, size_t size)
{
    if (!wasm_runtime_validate_native_addr(moduleInstance, nativePtr, size)) {
        std::string errorMsg = "Pointer out of WAMR's module address space";
        SPDLOG_ERROR(errorMsg);
        throw std::runtime_error(errorMsg);
    }
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

uint32_t WAMRWasmModule::nativePointerToWasm(void* nativePtr)
{
    return wasm_runtime_addr_native_to_app(moduleInstance, nativePtr);
}

uint32_t WAMRWasmModule::growMemory(uint32_t nBytes)
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

uint32_t WAMRWasmModule::shrinkMemory(uint32_t nBytes)
{

    SPDLOG_WARN("WAMR ignoring shrink memory");
    return 0;
}

uint32_t WAMRWasmModule::mmapMemory(uint32_t nBytes)
{
    return growMemory(nBytes);
}

size_t WAMRWasmModule::getMemorySizeBytes()
{
    auto aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return aotMem->cur_page_count * aotMem->num_bytes_per_page;
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

uint32_t WAMRWasmModule::mmapFile(uint32_t fp, uint32_t length)
{
    // TODO - implement
    return 0;
}
}
