#include <enclave/inside/native.h>
#include <wasm/WasmEnvironment.h>

#include <wasm_export.h>

namespace sgx {
static uint32_t getpid_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG_SGX("getpid");
    return FAKE_PID;
}

static uint32_t pclose_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG_SGX("pclose");
    UNIMPLEMENTED_WASM_INTRINSIC("pclose");
}

static uint32_t popen_wrapper(wasm_exec_env_t exec_env, uint32_t a, uint32_t b)
{
    SPDLOG_DEBUG_SGX("popen");
    UNIMPLEMENTED_WASM_INTRINSIC("popen");
}

static uint32_t raise_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG_SGX("raise");
    UNIMPLEMENTED_WASM_INTRINSIC("raise");
}

static uint32_t system_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG_SGX("system");
    UNIMPLEMENTED_WASM_INTRINSIC("system");
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(getpid, "()i"),  REG_NATIVE_FUNC(pclose, "(i)i"),
    REG_NATIVE_FUNC(popen, "(ii)i"), REG_NATIVE_FUNC(raise, "(i)i"),
    REG_NATIVE_FUNC(system, "(i)i"),
};

uint32_t getFaasmProcessApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
