#include "WAMRWasmModule.h"
#include "aot_runtime.h"
#include "faabric/util/logging.h"
#include "platform_common.h"
#include "wasm/WasmModule.h"
#include "wasm_exec_env.h"
#include "wasm_runtime.h"

#include <faabric/util/locks.h>
#include <wamr/native.h>

#include <cstdint>
#include <stdexcept>
#include <storage/FileLoader.h>
#include <wasm_export.h>

#if (WAMR_EXECUTION_MODE_INTERP)
#include <wasm_runtime.h>
#else
#include <aot_runtime.h>
#endif

namespace wasm {
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

        // Initialise runtime
        bool success = wasm_runtime_init();
        if (!success) {
            throw std::runtime_error("Failed to initialise WAMR");
        }

        SPDLOG_DEBUG("Successfully initialised WAMR");

        // Initialise native functions
        initialiseWAMRNatives();
    }
}

void tearDownWAMRGlobally()
{
    wasm_runtime_destroy();
}

WAMRWasmModule* getExecutingWAMRModule()
{
    return reinterpret_cast<WAMRWasmModule*>(getExecutingModule());
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

// ----- Module lifecycle -----
void WAMRWasmModule::doBindToFunction(const faabric::Message& msg, bool cache)
{

    // Prepare the filesystem
    filesystem.prepareFilesystem();

    // Load the wasm file
    storage::FileLoader& functionLoader = storage::getFileLoader();
#if (WAMR_EXECUTION_MODE_INTERP)
    wasmBytes = functionLoader.loadFunctionWasm(msg);
#else
    std::vector<uint8_t> wasmBytes =
      functionLoader.loadFunctionWamrAotFile(msg);
#endif

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

    setExecutingModule(this);
    setExecutingCall(&msg);

    // Run wasm initialisers
    executeWasmFunction(WASM_CTORS_FUNC_NAME);

    if (msg.funcptr() > 0) {
        // Run the function from the pointer
        executeWasmFunctionFromPointer(msg.funcptr());
    } else {
        // Run the main function
        executeWasmFunction(ENTRY_FUNC_NAME);
    }

    return 0;
}

int WAMRWasmModule::executeWasmFunctionFromPointer(int wasmFuncPtr)
{

    setExecutingModule(this);

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

    setExecutingModule(this);

    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, funcName.c_str(), nullptr);

    // Note, for some reason WAMR sets the return value in the argv array you
    // pass it, therefore we should provide a single integer argv even though
    // it's not actually used
    std::vector<uint32_t> argv = { 0 };

    // Invoke the function
#if (WAMR_EXECUTION_MODE_INTERP)
    bool success = wasm_create_exec_env_and_call_function(
      reinterpret_cast<WASMModuleInstance*>(moduleInstance),
      reinterpret_cast<WASMFunctionInstance*>(func),
      0x0,
      argv.data());
#else
    bool success = aot_create_exec_env_and_call_function(
      reinterpret_cast<AOTModuleInstance*>(moduleInstance),
      reinterpret_cast<AOTFunctionInstance*>(func),
      0x0,
      argv.data());
#endif

    // Check function result
    if (success) {
        SPDLOG_DEBUG("{} finished", funcName);
    } else {
#if (WAMR_EXECUTION_MODE_INTERP)
        std::string errorMessage(
          ((WASMModuleInstance*)moduleInstance)->cur_exception);
#else
        std::string errorMessage(
          ((AOTModuleInstance*)moduleInstance)->cur_exception);
#endif
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

size_t WAMRWasmModule::getMemorySizeBytes()
{
#if (WAMR_EXECUTION_MODE_INTERP)
    auto interpModule = reinterpret_cast<WASMModuleInstance*>(moduleInstance);
    WASMMemoryInstance* interpMem =
      ((WASMMemoryInstance**)interpModule->memories)[0];
    return interpMem->cur_page_count * interpMem->num_bytes_per_page;
#else
    auto aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem =
      ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
    return aotMem->cur_page_count * aotMem->num_bytes_per_page;
#endif
}

uint32_t WAMRWasmModule::mmapFile(uint32_t fp, uint32_t length)
{
    // TODO - implement
    return 0;
}
}
