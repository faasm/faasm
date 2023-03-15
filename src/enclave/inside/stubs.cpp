#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

namespace sgx {
// ------------------------------------------
// NOTE: All of these functions are just stubs to get things to run. We do not
// expect them to be called, so throw errors when they are.
// ------------------------------------------

static int32_t shm_open_wrapper(wasm_exec_env_t execEnv,
                                char* a,
                                int32_t b,
                                int32_t c)
{
    SPDLOG_DEBUG_SGX("S - shm_open");
    UNIMPLEMENTED_WASM_INTRINSIC("shm_open");
}

static int32_t syscall_wrapper(wasm_exec_env_t execEnv,
                               int32_t syscallNo,
                               int32_t syscallArgs)
{
    SPDLOG_DEBUG_SGX("S - syscall %i", syscallNo);
    UNIMPLEMENTED_WASM_INTRINSIC("syscall");
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(shm_open, "($ii)i"),
    REG_NATIVE_FUNC(syscall, "(ii)i"),
};

uint32_t getFaasmStubs(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
