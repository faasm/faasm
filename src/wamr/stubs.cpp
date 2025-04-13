#include <faabric/util/logging.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm_export.h>

namespace wasm {
// ------------------------------------------
// NOTE: All of these functions are just stubs to get things to run. We do not
// expect them to be called, so throw errors when they are.
// ------------------------------------------

static int32_t syscall_wrapper(wasm_exec_env_t exec_env,
                               int32_t syscallNo,
                               int32_t syscallArgs)
{
    SPDLOG_DEBUG("syscall - {}", syscallNo);
    switch (syscallNo) {
        case 224:
            // We only support gettid here
            SPDLOG_WARN("Using syscall to call gettid (syscall no. {})",
                        syscallNo);
            return 0;
        default:
            WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("syscall");

            return 0;
    }
}

static int32_t __cxa_allocate_exception_wrapper(wasm_exec_env_t exec_env,
                                                int32_t a)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("__cxa_allocate_exception");

    return 0;
}

static void __cxa_throw_wrapper(wasm_exec_env_t exec_env,
                                int32_t a,
                                int32_t b,
                                int32_t c)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("__cxa_throw");
}

static int32_t shm_open_wrapper(wasm_exec_env_t exec_env,
                                char* a,
                                int32_t b,
                                int32_t c)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("shm_open");

    return 0;
}

static void __accless_get_attestation_jwt_wrapper(wasm_exec_env_t execEnv,
                                                  int32_t* jwtPtrPtr,
                                                  int32_t* jwtSizePtr)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("accless_get_attestation_jwt");
}

static void __accless_get_mrenclave_wrapper(wasm_exec_env_t execEnv,
                                            int32_t* buf,
                                            int32_t bufSize)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("accless_get_mrenclave");
}

static int32_t __accless_is_enabled_wrapper(wasm_exec_env_t execEnv)
{
    return 1;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__cxa_allocate_exception, "(i)i"),
    REG_NATIVE_FUNC(__cxa_throw, "(iii)"),
    REG_NATIVE_FUNC(shm_open, "($ii)i"),
    REG_NATIVE_FUNC(syscall, "(ii)i"),
    // This three symbols are only used in SGX, but given that the code-gen is
    // performed by the main WAMR runtime, we define the symbols here to
    // avoid warnings
    REG_NATIVE_FUNC(__accless_get_attestation_jwt, "(**)"),
    REG_NATIVE_FUNC(__accless_get_mrenclave, "(*i)"),
    REG_NATIVE_FUNC(__accless_is_enabled, "()i"),
};

uint32_t getFaasmStubs(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
