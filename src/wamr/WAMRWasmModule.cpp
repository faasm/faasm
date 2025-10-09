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
#include <wasm_export.h>
#include <wasm_runtime_common.h>

#define FAASM_WAMR_NUM_MAX_THREADS 200
#define NO_WASM_FUNC_PTR -1

namespace wasm {
// The high level API for WAMR can be found here:
// https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/core/iwasm/include/wasm_export.h
static bool wamrInitialised = false;
const std::string WASI_PROC_EXIT = "Exception: wasi proc exit";

// WAMR maintains some global state, which we must be careful not to modify
// concurrently from our side. We are deliberately cautious with this locking,
// so it may cause performance issues under high churn of short-lived functions.
static std::mutex wamrGlobalsMutex;

/* RAII wrapper aroudn WAMR's thread enviornment (de)-initialisation.
 *
 * When using WAMR as a library from multiple threads, WAMR complains if
 * different steps of the runtime's lifecycle (e.g. initialisation, code gen,
 * module loading, memory instantiation, or function execution) are performed
 * by different threads. To fix this issue, we must make sure that each
 * thread's environment is properly initialized and (very importantly!)
 * destroyed after that. Failing to destroy it will create memory leaks in
 * TLS, leading to hard-to-debug segmentation faults.
 */
class WAMRThreadEnv
{
  public:
    WAMRThreadEnv()
    {
        if (!wasm_runtime_thread_env_inited() &&
            !wasm_runtime_init_thread_env()) {
            SPDLOG_ERROR("Error intialisining thread env. for main thread");
            throw std::runtime_error("Error intiialising thread environment!");
        }
    }

    ~WAMRThreadEnv()
    {
        if (wasm_runtime_thread_env_inited()) {
            wasm_runtime_destroy_thread_env();
        }
    }
};

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
    initArgs.mem_alloc_option.allocator.malloc_func =
      (void*)faabric::util::malloc;
    initArgs.mem_alloc_option.allocator.realloc_func =
      (void*)faabric::util::realloc;
    initArgs.mem_alloc_option.allocator.free_func = (void*)faabric::util::free;
    initArgs.max_thread_num = FAASM_WAMR_NUM_MAX_THREADS;

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

void WAMRWasmModule::destroyWAMRGlobally()
{
    faabric::util::UniqueLock lock(wamrGlobalsMutex);

    if (!wamrInitialised) {
        return;
    }

    wasm_runtime_destroy();

    wamrInitialised = false;
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

    // For WAMR's deinitialization procedure see here:
    // https://github.com/bytecodealliance/wasm-micro-runtime/tree/main/doc/embed_wamr.md#the-deinitialization-procedure
    destroyThreadsExecEnv(true);
    wasm_runtime_deinstantiate(moduleInstance);
    wasm_runtime_unload(wasmModule);
}

WAMRWasmModule* getExecutingWAMRModule()
{
    return reinterpret_cast<WAMRWasmModule*>(getExecutingModule());
}

// ----- Module lifecycle -----

// Reset is called once, from the main thread
void WAMRWasmModule::reset(faabric::Message& msg,
                           const std::string& snapshotKey)
{
    if (!_isBound) {
        return;
    }

    std::string funcStr = faabric::util::funcToString(msg, true);
    SPDLOG_DEBUG("WAMR resetting after {} (snap key {})", funcStr, snapshotKey);

    destroyThreadsExecEnv(true);
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

    {
        faabric::util::UniqueLock lock(wamrGlobalsMutex);

        // Must load the WASM file with a lock to prevent some spurious race
        // conditions when migrating to a new host
        wasmBytes = functionLoader.loadFunctionWamrAotFile(msg);

        SPDLOG_TRACE("WAMR loading {} wasm bytes\n", wasmBytes.size());
        wasmModule = wasm_runtime_load(
          wasmBytes.data(), wasmBytes.size(), errorBuffer, ERROR_BUFFER_SIZE);

        if (wasmModule == nullptr) {
            std::string errorMsg = std::string(errorBuffer);
            SPDLOG_ERROR(
              "{}:{}:{} Failed to load WAMR module {}/{} (size: {}): \n{}",
              msg.appid(),
              msg.groupid(),
              msg.groupidx(),
              msg.user(),
              msg.function(),
              wasmBytes.size(),
              errorMsg);
            throw std::runtime_error("Failed to load WAMR module");
        }
    }

    bindInternal(msg);
}

void WAMRWasmModule::bindInternal(faabric::Message& msg)
{
    // Prepare the filesystem
    filesystem.prepareFilesystem();

    // RAII-handle around WAMR's thread environment
    WAMRThreadEnv threadEnv;

    // Instantiate module. Set the app-managed heap size to 0 to use
    // wasi-libc's managed heap. See:
    // https://bytecodealliance.github.io/wamr.dev/blog/understand-the-wamr-heap/
    moduleInstance = wasm_runtime_instantiate(
      wasmModule, STACK_SIZE_KB, 0, errorBuffer, ERROR_BUFFER_SIZE);

    if (moduleInstance == nullptr) {
        SPDLOG_ERROR("WAMR module instantiation failed: {}", errorBuffer);
        throw std::runtime_error("WAMR module instantiation failed!");
    }

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

    // In WAMR the thread stacks are managed by the runtime, not by us, in
    // a dynamic fashion that also guarantees no overflows. As a consequence,
    // we do not need to use Faasm's home-built thread stack management
    threadStacks = std::vector<uint32_t>(threadPoolSize, 0);

    // Create the execution environemnt for all the potential threads
    execEnvs = std::vector<WASMExecEnv*>(threadPoolSize, nullptr);

    // Execution environment for the main thread
    execEnvs.at(0) =
      wasm_runtime_create_exec_env(moduleInstance, STACK_SIZE_KB);
    if (execEnvs.at(0) == nullptr) {
        SPDLOG_ERROR("Error allocating execution environment for main thread!");
    }
}

void WAMRWasmModule::createThreadsExecEnv(WASMExecEnv* parentExecEnv)
{
    if (parentExecEnv != execEnvs.at(0)) {
        SPDLOG_ERROR(
          "Creating thread's execution environment from non-main thread!");
        throw std::runtime_error("Creating threads from non-main thread!");
    }

    for (int i = 1; i < threadPoolSize; i++) {
        if (execEnvs.at(i) == nullptr) {
            wasm_runtime_set_exec_env_tls(parentExecEnv);
            execEnvs.at(i) = wasm_runtime_spawn_exec_env(execEnvs.at(0));
        }
        if (execEnvs.at(i) == nullptr) {
            SPDLOG_ERROR(
              "Error allocating execution environment for thread {}!", i);
            throw std::runtime_error("Error allocating exec. env!");
        }
    }
}

// Note that exec envs [1, threadPoolSize) are created using the spawn API,
// whereas exec env 0 is created using the normal create/destroy API. As
// a consequence, each must be destroyed with the corresponding API
void WAMRWasmModule::destroyThreadsExecEnv(bool destroyMainExecEnv)
{
    for (size_t i = 1; i < execEnvs.size(); i++) {
        if (execEnvs.at(i) != nullptr) {
            wasm_runtime_destroy_spawned_exec_env(execEnvs.at(i));
            execEnvs.at(i) = nullptr;
        }
    }

    if (destroyMainExecEnv && execEnvs.at(0) != nullptr) {
        wasm_runtime_destroy_exec_env(execEnvs.at(0));
        execEnvs.at(0) = nullptr;
        wasm_runtime_set_exec_env_tls(nullptr);
    }
}

int32_t WAMRWasmModule::executeFunction(faabric::Message& msg)
{
    SPDLOG_DEBUG("WAMR executing message {}", msg.id());

    // If we are calling this function, we know we are thread pool 0
    int thisThreadPoolIdx = 0;

    // Make sure context is set
    WasmExecutionContext ctx(this);
    int returnValue = 0;

    if (msg.funcptr() > 0) {
        // Run the function from the pointer
        returnValue = executeWasmFunctionFromPointer(thisThreadPoolIdx, msg);
    } else {
        prepareArgcArgv(msg);

        // Run the main function
        returnValue = executeWasmFunction(thisThreadPoolIdx, ENTRY_FUNC_NAME);

        // When running the main function (_start in WASI) we want to overwrite
        // the function's return value for the one in WAMR's WASI context.
        // The former is just the return value of _start, whereas the latter
        // is the actual return value of the entrypoint (e.g. main)
        returnValue = wasm_runtime_get_wasi_ctx(moduleInstance)->exit_code;
    }

    // Record the return value
    msg.set_returnvalue(returnValue);

    return returnValue;
}

AOTFuncType* getFuncTypeFromFuncPtr(WASMModuleCommon* wasmModule,
                                    WASMModuleInstanceCommon* moduleInstance,
                                    int32_t wasmFuncPtr)
{
    assert(wasmModule != nullptr);
    assert(moduleInstance != nullptr);

    AOTModuleInstance* aotModuleInstance =
      reinterpret_cast<AOTModuleInstance*>(moduleInstance);
    // FIXME: this pointer is unaligned, triggering UB
    AOTTableInstance* tableInstance = aotModuleInstance->tables[0];
    if (tableInstance == nullptr ||
        (uint32_t)wasmFuncPtr >= tableInstance->cur_size) {
        SPDLOG_ERROR("Error getting WAMR function signature from ptr: {}",
                     wasmFuncPtr);
        throw std::runtime_error("Error getting WAMR function signature");
    }
    uint32_t funcIdx = tableInstance->elems[wasmFuncPtr];
    uint32_t funcTypeIdx = aotModuleInstance->func_type_indexes[funcIdx];

    AOTModule* aotModule = reinterpret_cast<AOTModule*>(wasmModule);

    return aotModule->types[funcTypeIdx];
}

int32_t WAMRWasmModule::executeOMPThread(int threadPoolIdx,
                                         uint32_t stackTop,
                                         faabric::Message& msg)
{
    auto funcStr = faabric::util::funcToString(msg, false);
    int wasmFuncPtr = msg.funcptr();
    SPDLOG_DEBUG("Executing OpenMP thread {} for {} (app: {}, funcptr: {}))",
                 threadPoolIdx,
                 funcStr,
                 msg.appid(),
                 wasmFuncPtr);

    auto* execEnv = execEnvs.at(threadPoolIdx);
    if (execEnvs.at(threadPoolIdx) == nullptr) {
        SPDLOG_ERROR("Exec. env not set for thread: {}:{} (app: {})!",
                     funcStr,
                     threadPoolIdx,
                     msg.appid());
        throw std::runtime_error("Thread execution environment not set!");
    }

    auto* moduleInstance = wasm_runtime_get_module_inst(execEnv);
    auto* wasmModule = wasm_runtime_get_module(moduleInstance);

    auto ompLevel = threads::getCurrentOpenMPLevel();
    if (ompLevel == nullptr) {
        SPDLOG_ERROR("null OMP level!");
    }

    // Set up function args
    // NOTE: an OpenMP microtask takes the following arguments:
    // - The thread ID within its current team
    // - The number of non-global shared variables it has access to
    // - A pointer to each of the non-global shared variables
    int argc = 2 + ompLevel->nSharedVarOffsets;
    std::vector<uint32_t> argv(argc);
    argv[0] = { (uint32_t)msg.appidx() };
    argv[1] = { (uint32_t)ompLevel->nSharedVarOffsets };

    // The rest of the arguments are the ones corresponding to OpenMP
    for (size_t i = 0; i < ompLevel->nSharedVarOffsets; i++) {
        argv.at(i + 2) = ompLevel->sharedVarOffsets[i];
    }

    // Work-out if the function reutrns a value or returns void
    AOTFuncType* funcType =
      getFuncTypeFromFuncPtr(wasmModule, moduleInstance, wasmFuncPtr);
    bool returnsVoid = funcType->result_count == 0;

    auto originalArgv = argv;
    bool success =
      executeCatchException(threadPoolIdx, nullptr, wasmFuncPtr, argc, argv);

    if (!success) {
        SPDLOG_ERROR("Error executing OpenMP thread {} func {}: {}",
                     threadPoolIdx,
                     wasmFuncPtr,
                     wasm_runtime_get_exception(moduleInstance));
        throw std::runtime_error(
          "Error executing WASM OpenMP func ptr with WAMR");
    }

    // If we are calling a void function by pointer with some arguments, the
    // return value will be, precisely, the input arguments (and not argv[0])
    uint32_t returnValue = argv[0];
    if (returnsVoid) {
        returnValue = !(argv[0] == originalArgv[0]);
    }

    SPDLOG_DEBUG(
      "WAMR finished executing OMP thread {} for {}", threadPoolIdx, funcStr);
    return returnValue;
}

int32_t WAMRWasmModule::executePthread(int threadPoolIdx,
                                       uint32_t stackTop,
                                       faabric::Message& msg)
{
    auto funcStr = faabric::util::funcToString(msg, false);
    int wasmFuncPtr = msg.funcptr();
    SPDLOG_DEBUG("WAMR executing Pthread {} for {}", threadPoolIdx, funcStr);

    auto* execEnv = execEnvs.at(threadPoolIdx);
    if (execEnvs.at(threadPoolIdx) == nullptr) {
        SPDLOG_ERROR("Exec. env not set for thread: {}!", threadPoolIdx);
        throw std::runtime_error("Thread execution environment not set!");
    }

    auto* moduleInstance = wasm_runtime_get_module_inst(execEnv);
    auto* wasmModule = wasm_runtime_get_module(moduleInstance);

    int argsPtr = std::stoi(msg.inputdata());
    std::vector<uint32_t> argv = { (uint32_t)argsPtr };

    // Work-out if the function reutrns a value or returns void
    AOTFuncType* funcType =
      getFuncTypeFromFuncPtr(wasmModule, moduleInstance, wasmFuncPtr);
    bool returnsVoid = funcType->result_count == 0;

    auto originalArgv = argv;
    bool success = executeCatchException(
      threadPoolIdx, nullptr, wasmFuncPtr, argv.size(), argv);

    if (!success) {
        SPDLOG_ERROR("Error executing Pthread func {}: {}",
                     wasmFuncPtr,
                     wasm_runtime_get_exception(moduleInstance));
        throw std::runtime_error(
          "Error executing WASM Pthread func ptr with WAMR");
    }

    // If we are calling a void function by pointer with some arguments, the
    // return value will be, precisely, the input arguments (and not argv[0])
    uint32_t returnValue = argv[0];
    if (returnsVoid) {
        returnValue = !(argv[0] == originalArgv[0]);
    }

    SPDLOG_DEBUG("WAMR finished executing Pthread thread {} for {} (ret: {})",
                 threadPoolIdx,
                 funcStr,
                 returnValue);
    return returnValue;
}

int WAMRWasmModule::executeWasmFunctionFromPointer(int threadPoolIdx,
                                                   faabric::Message& msg)
{
    // WASM function pointers are indices into the module's function table
    int wasmFuncPtr = msg.funcptr();
    std::string inputData = msg.inputdata();

    SPDLOG_DEBUG("WAMR executing function from pointer {} (args: {})",
                 wasmFuncPtr,
                 inputData);

    AOTFuncType* funcType =
      getFuncTypeFromFuncPtr(wasmModule, moduleInstance, wasmFuncPtr);
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
    bool success = executeCatchException(
      threadPoolIdx, nullptr, wasmFuncPtr, argCount, argv);

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

int WAMRWasmModule::executeWasmFunction(int threadPoolIdx,
                                        const std::string& funcName)
{
    SPDLOG_DEBUG("WAMR executing function from string {}", funcName);

    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, funcName.c_str());
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
    bool success =
      executeCatchException(threadPoolIdx, func, NO_WASM_FUNC_PTR, 0, argv);
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
bool WAMRWasmModule::executeCatchException(int threadPoolIdx,
                                           WASMFunctionInstanceCommon* func,
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

    // Prepare thread execution environment
    WASMExecEnv* thisThreadExecEnv = execEnvs.at(threadPoolIdx);
    if (thisThreadExecEnv == nullptr) {
        SPDLOG_ERROR("Null execution environment for thread: {}!",
                     threadPoolIdx);
        throw std::runtime_error("Null execution environment!");
    }

    // RAII handle around WAMR's thread env.
    WAMRThreadEnv threadEnv;

    // Initialise thread environment
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
                      thisThreadExecEnv, wasmFuncPtr, argc, argv.data());
                } else {
                    success = wasm_runtime_call_wasm(
                      thisThreadExecEnv, func, argc, argv.data());
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
            case WAMRExceptionTypes::FunctionFrozenException: {
                throw faabric::util::FunctionFrozenException(
                  "Freezing MPI rank");
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

    // Report "wasi proc exit" as success
    if (!success) {
        const char* exceptionPtr = wasm_runtime_get_exception(moduleInstance);
        if (exceptionPtr != nullptr &&
            (std::string(exceptionPtr) == WASI_PROC_EXIT)) {
            success = true;
        }
    }

    return success;
}

// -----
// Exception handling
// -----

void WAMRWasmModule::doThrowException(std::exception& exc)
{
    // Switch over the different exception types we support. Unfortunately,
    // the setjmp/longjmp mechanism to catch C++ exceptions only lets us
    // change the return value of setjmp, but we can't propagate the string
    // associated to the exception
    if (dynamic_cast<faabric::util::FunctionMigratedException*>(&exc) !=
        nullptr) {
        // Make sure to explicitly call the exceptions destructor explicitly
        // to avoid memory leaks when longjmp-ing
        exc.~exception();
        longjmp(wamrExceptionJmpBuf,
                WAMRExceptionTypes::FunctionMigratedException);
    } else if (dynamic_cast<faabric::util::FunctionFrozenException*>(&exc) !=
               nullptr) {
        // Make sure to explicitly call the exceptions destructor explicitly
        // to avoid memory leaks when longjmp-ing
        exc.~exception();
        longjmp(wamrExceptionJmpBuf,
                WAMRExceptionTypes::FunctionFrozenException);
    } else if (dynamic_cast<faabric::util::QueueTimeoutException*>(&exc) !=
               nullptr) {
        exc.~exception();
        longjmp(wamrExceptionJmpBuf, WAMRExceptionTypes::QueueTimeoutException);
    } else {
        exc.~exception();
        longjmp(wamrExceptionJmpBuf, WAMRExceptionTypes::DefaultException);
    }
}

// -----
// Helper functions
// -----

void WAMRWasmModule::writeWasmEnvToWamrMemory(uint32_t* envOffsetsWasm,
                                              char* envBuffWasm)
{
    writeStringArrayToMemory(
      wasmEnvironment.getVars(), envOffsetsWasm, envBuffWasm);
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

uint32_t WAMRWasmModule::mmapFile(uint32_t hostFd, size_t length)
{
    // Create a new memory region in WASM
    int32_t wasmOffset = mmapMemory(length);
    uint8_t* nativePtr = wasmPointerToNative(wasmOffset);

    // Instead of ::munmap + ::map with a fd (which is not too robust), just
    // read the file contents into our newly mmap-ed memory
    auto bytesRead = read(hostFd, nativePtr, length);
    if (bytesRead == -1) {
        SPDLOG_ERROR("Error reading from fd {}", hostFd);
        throw std::runtime_error("Error reading from file descriptor");
    }

    return wasmOffset;
}
}
