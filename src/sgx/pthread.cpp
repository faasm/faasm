#include <sgx/native.h>

namespace sgx {
static int32_t pthread_mutex_init_wrapper(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t b)
{
    return 0;
}

static int32_t pthread_mutex_lock_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    return 0;
}

static int32_t pthread_mutex_unlock_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    return 0;
}

static int32_t pthread_cond_broadcast_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a)
{
    return 0;
}

static int32_t pthread_mutexattr_init_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a)
{
    return 0;
}

static int32_t pthread_mutexattr_destroy_wrapper(wasm_exec_env_t exec_env,
                                                 int32_t a)
{
    return 0;
}

static int32_t pthread_equal_wrapper(wasm_exec_env_t exec_env,
                                     int32_t a,
                                     int32_t b)
{
    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(pthread_mutex_init, "(ii)i"),
    REG_NATIVE_FUNC(pthread_mutex_lock, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutex_unlock, "(i)i"),
    REG_NATIVE_FUNC(pthread_cond_broadcast, "(i)i"),
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
