#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

namespace sgx {

static int32_t dlclose_wrapper(wasm_exec_env_t exec_env, void* handle)
{
    SPDLOG_DEBUG_SGX("S - dlclose");
    UNIMPLEMENTED_WASM_INTRINSIC("dlclose");
}

static int32_t dlopen_wrapper(wasm_exec_env_t exec_env,
                              char* filename,
                              int32_t flags)
{
    SPDLOG_DEBUG_SGX("S - dlopen");
    UNIMPLEMENTED_WASM_INTRINSIC("dlopen");
}

static int32_t dlsym_wrapper(wasm_exec_env_t exec_env,
                             void* handle,
                             char* symbol)
{
    SPDLOG_DEBUG_SGX("S - dlsym");
    UNIMPLEMENTED_WASM_INTRINSIC("dlsym");
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(dlclose, "(*)i"),
    REG_NATIVE_FUNC(dlopen, "($i)i"),
    REG_NATIVE_FUNC(dlsym, "(*$)i"),
};

uint32_t getFaasmDynlinkApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
