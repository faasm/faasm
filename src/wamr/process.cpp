#include <faabric/util/logging.h>
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
    SPDLOG_DEBUG("pclose");
    throw std::runtime_error("pclose not implemented");
}

static uint32_t popen_wrapper(wasm_exec_env_t exec_env, uint32_t a, uint32_t b)
{
    SPDLOG_DEBUG("popen");
    throw std::runtime_error("popen not implemented");
}

static uint32_t raise_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG("raise");
    throw std::runtime_error("raise not implemented");
}

static uint32_t system_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG("system");
    throw std::runtime_error("system not implemented");
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
