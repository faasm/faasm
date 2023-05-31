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
#include <setjmp.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <aot_runtime.h>
#include <platform_common.h>
#include <wasm_exec_env.h>
#include <wasm_export.h>

#define NO_WASM_FUNC_PTR -1

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
    SPDLOG_DEBUG("WAMR resetting after {} (snap key {})", funcStr, snapshotKey);

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
    createThreadStacks();
}

int32_t WAMRWasmModule::executeFunction(faabric::Message& msg)
{
    SPDLOG_DEBUG("WAMR executing message {}", msg.id());

    // Make sure context is set
    WasmExecutionContext ctx(this);
    int returnValue = 0;

    if (msg.funcptr() > 0) {
        // Run the function from the pointer
        returnValue = executeWasmFunctionFromPointer(msg);
    } else {
        prepareArgcArgv(msg);

        // Run the main function
        returnValue = executeWasmFunction(ENTRY_FUNC_NAME);
    }

    // Record the return value
    msg.set_returnvalue(returnValue);

    return returnValue;
}

int WAMRWasmModule::executeWasmFunctionFromPointer(faabric::Message& msg)
{
    // WASM function pointers are indices into the module's function table
    int wasmFuncPtr = msg.funcptr();
    std::string inputData = msg.inputdata();

    SPDLOG_DEBUG("WAMR executing function from pointer {} (args: {})",
                 wasmFuncPtr,
                 inputData);

    // Work out the function signature from the function pointer
    AOTModuleInstance* aotModuleInstance =
      reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    AOTTableInstance* tableInstance = aotModuleInstance->tables[0];
    if (tableInstance == nullptr || wasmFuncPtr >= tableInstance->cur_size) {
        SPDLOG_ERROR("Error getting WAMR function signature from ptr: {}",
                     wasmFuncPtr);
        throw std::runtime_error("Error getting WAMR function signature");
    }
    uint32_t funcIdx = tableInstance->elems[wasmFuncPtr];
    uint32_t funcTypeIdx = aotModuleInstance->func_type_indexes[funcIdx];

    AOTModule* aotModule = reinterpret_cast<AOTModule*>(wasmModule);
    AOTFuncType* funcType = aotModule->func_types[funcTypeIdx];
    int argCount = funcType->param_count;
    int resultCount = funcType->result_count;
    SPDLOG_DEBUG("WAMR Function pointer has {} arguments and returns {} value",
                 argCount,
                 resultCount);
    bool returnsVoid = resultCount == 0;

    // NOTE: WAMR doesn't provide a nice interface for calling functions using
    // function pointers, so we have to call a few more low-level functions to
    // get it to work.

    std::vector<uint32_t> argv;
    switch (argCount) {
        // Even if the function takes no arguments, we need to pass an argv
        // with at least one element, as WAMR will set the return value in
        // argv[0]
        case 0:
            argv = { 0 };
            break;
        // If we are calling with just one argument, we assume its an integer
        // value. We could switch on the data type of the AOTType*, but we
        // don't do it just yet
        case 1:
            argv = { (uint32_t)std::stoi(inputData) };
            break;
        default: {
            SPDLOG_ERROR("Unrecognised WAMR function pointer signature (args: "
                         "{}, return: {})",
                         argCount,
                         resultCount);
            throw std::runtime_error(
              "Unrecognised WAMR function pointer signature");
        }
    }
    std::vector<uint32_t> originalArgv = argv;
    bool success = executeCatchException(nullptr, wasmFuncPtr, argCount, argv);

    if (!success) {
        SPDLOG_ERROR("Error executing {}: {}",
                     wasmFuncPtr,
                     wasm_runtime_get_exception(moduleInstance));
        throw std::runtime_error("Error executing WASM func ptr with WAMR");
    }

    // If we are calling a void function by pointer with some arguments, the
    // return value will be, precisely, the input arguments (and not 0)
    uint32_t returnValue;
    if (returnsVoid) {
        returnValue = !(argv[0] == originalArgv[0]);
    } else {
        returnValue = 0;
    }

    SPDLOG_DEBUG("WAMR finished executing func ptr {}", wasmFuncPtr);
    return returnValue;
}

int WAMRWasmModule::executeWasmFunction(const std::string& funcName)
{
    SPDLOG_DEBUG("WAMR executing function from string {}", funcName);

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
    bool success = executeCatchException(func, NO_WASM_FUNC_PTR, 0, argv);
    uint32_t returnValue = argv[0];

    if (!success) {
        SPDLOG_ERROR("Error executing {}: {}",
                     funcName,
                     wasm_runtime_get_exception(moduleInstance));
        throw std::runtime_error("Error executing WASM function with WAMR");
    }

    SPDLOG_DEBUG("WAMR finished executing {}", funcName);
    return returnValue;
}

// Low-level method to call a WASM function in WAMR and catch any thrown
// exceptions. This method is shared both if we call a function by pointer or
// by name
bool WAMRWasmModule::executeCatchException(WASMFunctionInstanceCommon* func,
                                           int wasmFuncPtr,
                                           int argc,
                                           std::vector<uint32_t>& argv)
{
    bool isIndirect;
    if (wasmFuncPtr == NO_WASM_FUNC_PTR && func != nullptr) {
        isIndirect = false;
    } else if (wasmFuncPtr != NO_WASM_FUNC_PTR && func == nullptr) {
        isIndirect = true;
    } else {
        throw std::runtime_error(
          "Incorrect combination of arguments to execute WAMR function");
    }

    auto execEnvDtor = [&](WASMExecEnv* execEnv) {
        if (execEnv != nullptr) {
            wasm_runtime_destroy_exec_env(execEnv);
        }
        wasm_runtime_set_exec_env_tls(nullptr);
    };

    // Create an execution environment
    std::unique_ptr<WASMExecEnv, decltype(execEnvDtor)> execEnv(
      wasm_exec_env_create(moduleInstance, STACK_SIZE_KB), execEnvDtor);
    if (execEnv == nullptr) {
        throw std::runtime_error("Error creating execution environment");
    }

    // Set thread handle and stack boundary (required by WAMR)
    wasm_exec_env_set_thread_info(execEnv.get());

    bool success;
    {
        // This switch statement is used to catch exceptions thrown by native
        // functions (written in C++) called from WASM code executed in WAMR.
        // Given that WAMR is written in C, exceptions are not propagated, and
        // thus we implement our custom handler
        switch (setjmp(wamrExceptionJmpBuf)) {
            case 0: {
                if (isIndirect) {
                    success = wasm_runtime_call_indirect(
                      execEnv.get(), wasmFuncPtr, argc, argv.data());
                } else {
                    success = wasm_runtime_call_wasm(
                      execEnv.get(), func, argc, argv.data());
                }
                break;
            }
            // Make sure that we throw an exception if setjmp is called from
            // a longjmp (and returns a value different than 0) as local
            // variables in the stack could be corrupted
            case WAMRExceptionTypes::FunctionMigratedException: {
                throw faabric::util::FunctionMigratedException(
                  "Migrating MPI rank");
            }
            case WAMRExceptionTypes::QueueTimeoutException: {
                throw std::runtime_error("Timed-out dequeueing!");
            }
            case WAMRExceptionTypes::DefaultException: {
                throw std::runtime_error("Default WAMR exception");
            }
            default: {
                SPDLOG_ERROR("WAMR exception handler reached unreachable case");
                throw std::runtime_error("Unreachable WAMR exception handler");
            }
        }
    }

    return success;
}

// -----
// Exception handling
// -----

void WAMRWasmModule::doThrowException(std::exception& e)
{
    // Switch over the different exception types we support. Unfortunately,
    // the setjmp/longjmp mechanism to catch C++ exceptions only lets us
    // change the return value of setjmp, but we can't propagate the string
    // associated to the exception
    if (dynamic_cast<faabric::util::FunctionMigratedException*>(&e) !=
        nullptr) {
        // Make sure to explicitly call the exceptions destructor explicitly
        // to avoid memory leaks when longjmp-ing
        e.~exception();
        longjmp(wamrExceptionJmpBuf,
                WAMRExceptionTypes::FunctionMigratedException);
    } else if (dynamic_cast<faabric::util::QueueTimeoutException*>(&e) !=
               nullptr) {
        e.~exception();
        longjmp(wamrExceptionJmpBuf, WAMRExceptionTypes::QueueTimeoutException);
    } else {
        e.~exception();
        longjmp(wamrExceptionJmpBuf, WAMRExceptionTypes::DefaultException);
    }
}

// -----
// Helper functions
// -----

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
    auto* aotModule = reinterpret_cast<AOTModuleInstance*>(moduleInstance);
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
