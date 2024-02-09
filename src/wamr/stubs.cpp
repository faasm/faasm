#include <faabric/util/logging.h>
#include <stdexcept>
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

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__cxa_allocate_exception, "(i)i"),
    REG_NATIVE_FUNC(__cxa_throw, "(iii)"),
    REG_NATIVE_FUNC(shm_open, "($ii)i"),
    REG_NATIVE_FUNC(syscall, "(ii)i"),
};

uint32_t getFaasmStubs(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
