#include <faabric/util/logging.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/threads.h>

#include <wasm_export.h>

namespace wasm {

static int32_t pthread_create_wrapper(wasm_exec_env_t execEnv,
                                      int32_t pthreadPtr,
                                      int32_t attrPtr,
                                      int32_t entryFunc,
                                      int32_t argsPtr)
{
    auto* wamrModule = wasm::getExecutingWAMRModule();
    wamrModule->createThreadsExecEnv(execEnv);

    // Set-up the wasm_pthread pointer
    wamrModule->validateWasmOffset(pthreadPtr, sizeof(wasm_pthread));
    wasm_pthread* nativePthreadPtr =
      (wasm_pthread*)wamrModule->wasmOffsetToNativePointer(pthreadPtr);
    nativePthreadPtr->selfPtr = pthreadPtr;

    return wasm::doPthreadCreate(pthreadPtr, attrPtr, entryFunc, argsPtr);
}

static void pthread_exit_wrapper(wasm_exec_env_t execEnv, int32_t code)
{
    wasm::doPthreadExit();
}

static int32_t pthread_join_wrapper(wasm_exec_env_t execEnv,
                                    int32_t pthreadPtr,
                                    int32_t* resPtr)
{
    int returnValue = wasm::doPthreadJoin(pthreadPtr);

    // Check that the caller is not ignoring the return value by passing null
    if (resPtr != nullptr) {
        *resPtr = returnValue;
    }

    // Our pthread_join implementation assumes that the first pthread_join
    // is called after _all_ pthread_create calls have been made, and waits
    // for _all_ pthread threads to finish. As a consequence, it is safe to
    // kill all thread execution environments from _any_ call to pthread_join
    // (the method is idempotent, so we can call many times)
    auto* wamrModule = getExecutingWAMRModule();
    wamrModule->destroyThreadsExecEnv();

    return 0;
}

static int32_t pthread_once_wrapper(wasm_exec_env_t exec_env,
                                    int32_t a,
                                    int32_t b)
{
    // Trace logging as this may be called a lot
    SPDLOG_TRACE("S - pthread_once {} {}", a, b);
    return 0;
}

static int32_t pthread_mutex_init_wrapper(wasm_exec_env_t execEnv,
                                          int32_t mutex,
                                          int32_t attr)
{
    SPDLOG_DEBUG("S - pthread_mutex_init {} {}", mutex, attr);

    return 0;
}

static int32_t pthread_mutex_lock_wrapper(wasm_exec_env_t execEnv,
                                          int32_t mutex)
{
    return doPthreadMutexLock(mutex);
}

static int32_t pthread_mutex_unlock_wrapper(wasm_exec_env_t execEnv,
                                            int32_t mutex)
{
    return doPthreadMutexUnlock(mutex);
}

static int32_t pthread_mutex_destroy_wrapper(wasm_exec_env_t execEnv,
                                             int32_t mutex)
{
    SPDLOG_DEBUG("S - pthread_mutex_destroy {}", mutex);
    return 0;
}

static int32_t pthread_cond_init_wrapper(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int32_t b)
{
    SPDLOG_DEBUG("S - pthread_cond_init {}", a, b);
    return 0;
}

static int32_t pthread_cond_signal_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    SPDLOG_DEBUG("S - pthread_cond_signal {}", a);
    return 0;
}

static int32_t pthread_cond_wait_wrapper(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int32_t b)
{
    SPDLOG_DEBUG("S - pthread_cond_wait {}", a, b);
    return 0;
}

static int32_t pthread_cond_broadcast_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a)
{
    SPDLOG_DEBUG("S - pthread_cond_broadcast {}", a);
    return 0;
}

static int32_t pthread_cond_destroy_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    SPDLOG_DEBUG("S - pthread_cond_destroy {}", a);
    return 0;
}

static int32_t pthread_mutexattr_init_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a)
{
    SPDLOG_DEBUG("S - pthread_mutexattr_init {}", a);
    return 0;
}

static int32_t pthread_mutexattr_destroy_wrapper(wasm_exec_env_t exec_env,
                                                 int32_t a)
{
    SPDLOG_DEBUG("S - pthread_mutexattr_destroy {}", a);
    return 0;
}

static int32_t pthread_equal_wrapper(wasm_exec_env_t exec_env,
                                     int32_t a,
                                     int32_t b)
{
    SPDLOG_DEBUG("S - pthread_equal {} {}", a, b);
    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(pthread_create, "(iiii)i"),
    REG_NATIVE_FUNC(pthread_exit, "(i)"),
    REG_NATIVE_FUNC(pthread_join, "(i*)i"),
    REG_NATIVE_FUNC(pthread_once, "(ii)i"),
    REG_NATIVE_FUNC(pthread_mutex_init, "(ii)i"),
    REG_NATIVE_FUNC(pthread_mutex_lock, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutex_unlock, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutex_destroy, "(i)i"),
    REG_NATIVE_FUNC(pthread_cond_init, "(ii)i"),
    REG_NATIVE_FUNC(pthread_cond_signal, "(i)i"),
    REG_NATIVE_FUNC(pthread_cond_wait, "(ii)i"),
    REG_NATIVE_FUNC(pthread_cond_broadcast, "(i)i"),
    REG_NATIVE_FUNC(pthread_cond_destroy, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutexattr_init, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutexattr_destroy, "(i)i"),
    REG_NATIVE_FUNC(pthread_equal, "(ii)i"),
};

uint32_t getFaasmPthreadApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
