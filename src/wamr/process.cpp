#include <faabric/util/logging.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmEnvironment.h>

#include <wasm_export.h>

namespace wasm {
static uint32_t getpid_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG("getpid");
    return FAKE_PID;
}

static uint32_t pclose_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("pclose");

    return 0;
}

static uint32_t popen_wrapper(wasm_exec_env_t exec_env, uint32_t a, uint32_t b)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("popen");

    return 0;
}

static uint32_t raise_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("raise");

    return 0;
}

static uint32_t system_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("system");

    return 0;
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
