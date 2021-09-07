#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <storage/FileLoader.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

#include <cstdint>
#include <stdexcept>

// WAMR includes
#include <aot_runtime.h>
#include <platform_common.h>
#include <wasm_exec_env.h>
#include <wasm_export.h>
#include <wasm_runtime.h>

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

        // Initialise WAMR runtime:
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

    currentBrk = getMemorySizeBytes();

    // Set up thread stacks
    createThreadStacks();
}

int32_t WAMRWasmModule::executeFunction(faabric::Message& msg)
{

    SPDLOG_DEBUG("WAMR executing message {}", msg.id());

    // Make sure context is set
    WasmExecutionContext ctx(this, &msg);

    // Run wasm initialisers
    executeWasmFunction(WASM_CTORS_FUNC_NAME);

    if (msg.funcptr() > 0) {
        // Run the function from the pointer
        executeWasmFunctionFromPointer(msg.funcptr());
    } else {
        prepareArgcArgv(msg);

        // Run the main function
        executeWasmFunction(ENTRY_FUNC_NAME);
    }

    return 0;
}

int WAMRWasmModule::executeWasmFunctionFromPointer(int wasmFuncPtr)
{

    // NOTE: WAMR doesn't provide a nice interface for calling functions using
    // function pointers, so we have to call a few more low-level functions to
    // get it to work.

    WASMExecEnv* execEnv = wasm_exec_env_create(moduleInstance, STACK_SIZE_KB);
    if (execEnv == nullptr) {
        SPDLOG_ERROR("Failed to create exec env for func ptr {}", wasmFuncPtr);
        throw std::runtime_error("Failed to create WAMR exec env");
    }

    // Set thread handle and stack boundary (required by WAMR)
    wasm_exec_env_set_thread_info(execEnv);

    // Call the function pointer
    // NOTE: for some reason WAMR uses the argv array to pass the function
    // return value, so we have to provide something big enough
    std::vector<uint32_t> argv = { 0 };
    bool success =
      wasm_runtime_call_indirect(execEnv, wasmFuncPtr, 0, argv.data());

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
        SPDLOG_ERROR("Function failed: {}", errorMessage);

        return 1;
    }

    return 0;
}

uint32_t WAMRWasmModule::growMemory(uint32_t nBytes)
{

    SPDLOG_DEBUG("WAMR growing memory by {}", nBytes);

    uint32_t memBase = currentBrk;

    uint32_t nPages = getNumberOfWasmPagesForBytes(nBytes);
    bool success = wasm_runtime_enlarge_memory(moduleInstance, nPages);
    if (!success) {
        throw std::runtime_error("Failed to grow WAMR memory");
    }

    currentBrk = memBase + (nPages * WASM_BYTES_PER_PAGE);
    return memBase;
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

uint8_t* WAMRWasmModule::wasmPointerToNative(int32_t wasmPtr)
{
    void* nativePtr = wasm_runtime_addr_app_to_native(moduleInstance, wasmPtr);
    return static_cast<uint8_t*>(nativePtr);
}

int32_t WAMRWasmModule::nativePointerToWasm(uint8_t* nativePtr)
{
    return wasm_runtime_addr_native_to_app(moduleInstance, nativePtr);
}

size_t WAMRWasmModule::getMemorySizeBytes()
{
    auto aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return aotMem->cur_page_count * aotMem->num_bytes_per_page;
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
