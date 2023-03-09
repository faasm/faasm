#include <wamr/native.h>
#include <wasm_export.h>

#include <faabric/util/logging.h>

namespace wasm {

// -------------------------------------------
// 14/04/21 - WAMR threading not implemented
// All of these functions are stubbed as threading with WAMR isn't yet
// implemented. Once it is, we will need to implement the function here,
// specially the locking which is used to manage thread-safe memory
// provisioning.
// -------------------------------------------

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

static int32_t pthread_create_wrapper(wasm_exec_env_t exec_env,
                                      int32_t a,
                                      int32_t b,
                                      int32_t c,
                                      int32_t d)
{
    SPDLOG_DEBUG("S - pthread_create {} {} {} {}", a, b, c, d);
    return 0;
}

static int32_t pthread_equal_wrapper(wasm_exec_env_t exec_env,
                                     int32_t a,
                                     int32_t b)
{
    SPDLOG_DEBUG("S - pthread_equal {} {}", a, b);
    return 0;
}

static int32_t pthread_join_wrapper(wasm_exec_env_t exec_env,
                                    int32_t a,
                                    int32_t b)
{
    SPDLOG_DEBUG("S - pthread_join {} {}", a, b);
    return 0;
}

static int32_t pthread_key_create_wrapper(wasm_exec_env_t execEnv,
                                          int32_t* key,
                                          int32_t destructorElemIdx)
{
    SPDLOG_DEBUG("S - pthread_key_create_wrapper");
    throw std::runtime_error("Unimplemented pthread_key_create_wrapper");
}

static int32_t pthread_mutex_destroy_wrapper(wasm_exec_env_t exec_env,
                                             int32_t a)
{
    SPDLOG_DEBUG("S - pthread_mutex_destroy {}", a);
    return 0;
}

static int32_t pthread_mutex_init_wrapper(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t b)
{
    SPDLOG_DEBUG("S - pthread_mutex_init {} {}", a, b);
    return 0;
}

static int32_t pthread_mutex_lock_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    SPDLOG_TRACE("S - pthread_mutex_lock {}", a);
    return 0;
}

static int32_t pthread_mutex_unlock_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    SPDLOG_TRACE("S - pthread_mutex_unlock {}", a);
    return 0;
}

static int32_t pthread_mutexattr_destroy_wrapper(wasm_exec_env_t exec_env,
                                                 int32_t a)
{
    SPDLOG_DEBUG("S - pthread_mutexattr_destroy {}", a);
    return 0;
}

static int32_t pthread_mutexattr_init_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a)
{
    SPDLOG_DEBUG("S - pthread_mutexattr_init {}", a);
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

static int32_t pthread_self_wrapper(wasm_exec_env_t execEnv)
{
    SPDLOG_DEBUG("S - pthread_self");
    throw std::runtime_error("Unimplemented: pthread_self");
}

static int32_t pthread_setspecific_wrapper(wasm_exec_env_t execEnv,
                                           int32_t key,
                                           int32_t value_offset)
{
    SPDLOG_DEBUG("S - pthread_setspecific");
    throw std::runtime_error("Unimplemented: pthread_setspecific");
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
