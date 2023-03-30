#include <faabric/util/files.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>
#include <storage/FileLoader.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

#include <cstdint>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <aot_runtime.h>
#include <platform_common.h>
#include <wasm_exec_env.h>
#include <wasm_export.h>

#include <aot_runtime.h>

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
    RuntimeInitArgs initArgs;
    memset(&initArgs, 0, sizeof(RuntimeInitArgs));

    // Memory configuration
    initArgs.mem_alloc_type = Alloc_With_Allocator;
    initArgs.mem_alloc_option.allocator.malloc_func = (void*)::malloc;
    initArgs.mem_alloc_option.allocator.realloc_func = (void*)::realloc;
    initArgs.mem_alloc_option.allocator.free_func = (void*)::free;

    bool success = wasm_runtime_full_init(&initArgs);
    if (!success) {
        throw std::runtime_error("Failed to initialise WAMR");
    }

    SPDLOG_DEBUG("Successfully initialised WAMR");

    // Initialise Faasm's own native symbols
    initialiseWAMRNatives();

    // Set log level: BH_LOG_LEVEL_{FATAL,ERROR,WARNING,DEBUG,VERBOSE}
    bh_log_set_verbose_level(BH_LOG_LEVEL_WARNING);

    wamrInitialised = true;
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
    SPDLOG_INFO("WAMR resetting after {} (snap key {})", funcStr, snapshotKey);

    wasm_runtime_deinstantiate(moduleInstance);
    bindInternal(msg);
}

void WAMRWasmModule::doBindToFunction(faabric::Message& msg, bool cache)
{
    SPDLOG_TRACE("WAMR binding to {}/{} via message {}",
                 msg.user(),
                 msg.function(),
                 msg.id());

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
    // Prepare the filesystem
    filesystem.prepareFilesystem();

    // Instantiate module. Set the app-managed heap size to 0 to use
    // wasi-libc's managed heap. See:
    // https://bytecodealliance.github.io/wamr.dev/blog/understand-the-wamr-heap/
    moduleInstance = wasm_runtime_instantiate(
      wasmModule, STACK_SIZE_KB, 0, errorBuffer, ERROR_BUFFER_SIZE);

    // Sense-check the module
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];

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
    // createThreadStacks();
    threadStacks.push_back(-1);
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
    SPDLOG_DEBUG("WAMR executing function from string {}", funcName);

    WASMExecEnv* execEnv = wasm_runtime_get_exec_env_singleton(moduleInstance);
    if (execEnv == nullptr) {
        SPDLOG_ERROR("Failed to create exec env for func {}", funcName);
        throw std::runtime_error("Failed to create WAMR exec env");
    }

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
    bool success = wasm_runtime_call_wasm(execEnv, func, 0, argv.data());
    uint32_t returnValue = argv[0];

    if (!success) {
        SPDLOG_ERROR("Error executing {}: {}", funcName, wasm_runtime_get_exception(moduleInstance));
        throw std::runtime_error("Error executing WASM function with WAMR");
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

bool WAMRWasmModule::doGrowMemory(uint32_t pageChange)
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    return aot_enlarge_memory(aotModule, pageChange);
}

size_t WAMRWasmModule::getMemorySizeBytes()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];
    return aotMem->cur_page_count * WASM_BYTES_PER_PAGE;
}

uint8_t* WAMRWasmModule::getMemoryBase()
{
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];
    return reinterpret_cast<uint8_t*>(aotMem->memory_data);
}

size_t WAMRWasmModule::getMaxMemoryPages()
{
    auto aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTMemoryInstance* aotMem = ((AOTMemoryInstance**)aotModule->memories)[0];
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
