#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

namespace sgx {
static int32_t pthread_cond_broadcast_wrapper(wasm_exec_env_t execEnv,
                                              int32_t a)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_broadcast_wrapper");
    return 0;
}

static int32_t pthread_cond_destroy_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_destroy");
    return 0;
}

static int32_t pthread_cond_init_wrapper(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int32_t b)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_init");
    return 0;
}

static int32_t pthread_cond_signal_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_signal");
    return 0;
}

static int32_t pthread_cond_wait_wrapper(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int32_t b)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_wait");
    return 0;
}

static int32_t pthread_create_wrapper(wasm_exec_env_t exec_env,
                                      int32_t a,
                                      int32_t b,
                                      int32_t c,
                                      int32_t d)
{
    SPDLOG_DEBUG_SGX("S - pthread_create");
    return 0;
}

static int32_t pthread_equal_wrapper(wasm_exec_env_t execEnv,
                                     int32_t a,
                                     int32_t b)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_equal_wrapper");
    return 0;
}

static int32_t pthread_join_wrapper(wasm_exec_env_t exec_env,
                                    int32_t a,
                                    int32_t b)
{
    SPDLOG_DEBUG_SGX("S - pthread_join");
    return 0;
}

static int32_t pthread_key_create_wrapper(wasm_exec_env_t execEnv,
                                          int32_t* key,
                                          int32_t destructorElemIdx)
{
    SPDLOG_DEBUG_SGX("S - pthread_key_create_wrapper");
    UNIMPLEMENTED_WASM_INTRINSIC("pthread_key_create_wrapper");
}

static int32_t pthread_mutex_destroy_wrapper(wasm_exec_env_t exec_env,
                                             int32_t a)
{
    SPDLOG_DEBUG_SGX("S - pthread_mutex_destroy");
    return 0;
}

static int32_t pthread_mutex_init_wrapper(wasm_exec_env_t execEnv,
                                          int32_t a,
                                          int32_t b)
{
    SPDLOG_DEBUG_SGX("S - pthread_mutex_init_wrapper");
    return 0;
}

static int32_t pthread_mutex_lock_wrapper(wasm_exec_env_t execEnv, int32_t a)
{
    // SPDLOG_DEBUG_SGX("S - pthread_mutex_lock_wrapper");
    return 0;
}

static int32_t pthread_mutex_unlock_wrapper(wasm_exec_env_t execEnv, int32_t a)
{
    // TODO(csegarragonz): add SPDLOG_TRACE_SGX
    // SPDLOG_DEBUG_SGX("S - pthread_unmutex_unlock_wrapper");
    return 0;
}

static int32_t pthread_mutexattr_init_wrapper(wasm_exec_env_t execEnv,
                                              int32_t a)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_init_wrapper");
    return 0;
}

static int32_t pthread_mutexattr_destroy_wrapper(wasm_exec_env_t execEnv,
                                                 int32_t a)
{
    SPDLOG_DEBUG_SGX("S - pthread_cond_destroy_wrapper");
    return 0;
}

static int32_t pthread_once_wrapper(wasm_exec_env_t exec_env,
                                    int32_t a,
                                    int32_t b)
{
    // Trace logging as this may be called a lot
    // SPDLOG_DEBUG_SGX("S - pthread_once");
    return 0;
}

static int32_t pthread_self_wrapper(wasm_exec_env_t execEnv)
{
    SPDLOG_DEBUG_SGX("S - pthread_self");
    UNIMPLEMENTED_WASM_INTRINSIC("pthread_self");
}

static int32_t pthread_setspecific_wrapper(wasm_exec_env_t execEnv,
                                           int32_t key,
                                           int32_t value_offset)
{
    SPDLOG_DEBUG_SGX("S - pthread_setspecific");
    UNIMPLEMENTED_WASM_INTRINSIC("pthread_setspecific");
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(pthread_cond_broadcast, "(i)i"),
    REG_NATIVE_FUNC(pthread_cond_destroy, "(i)i"),
    REG_NATIVE_FUNC(pthread_cond_init, "(ii)i"),
    REG_NATIVE_FUNC(pthread_cond_signal, "(i)i"),
    REG_NATIVE_FUNC(pthread_cond_wait, "(ii)i"),
    REG_NATIVE_FUNC(pthread_create, "(iiii)i"),
    REG_NATIVE_FUNC(pthread_equal, "(ii)i"),
    REG_NATIVE_FUNC(pthread_join, "(ii)i"),
    REG_NATIVE_FUNC(pthread_key_create, "(*i)i"),
    REG_NATIVE_FUNC(pthread_mutex_destroy, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutex_init, "(ii)i"),
    REG_NATIVE_FUNC(pthread_mutex_lock, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutex_unlock, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutexattr_init, "(i)i"),
    REG_NATIVE_FUNC(pthread_mutexattr_destroy, "(i)i"),
    REG_NATIVE_FUNC(pthread_once, "(ii)i"),
    REG_NATIVE_FUNC(pthread_self, "()i"),
    REG_NATIVE_FUNC(pthread_setspecific, "(ii)i"),
};

uint32_t getFaasmPthreadApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
