#include <stdexcept>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm_export.h>

namespace wasm {
// TODO - implement dynamic linking with WAMR

static int32_t dlopen_wrapper(wasm_exec_env_t exec_env,
                              char* filename,
                              int32_t flags)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("dlopen");

    return 0;
}

static int32_t dlsym_wrapper(wasm_exec_env_t exec_env,
                             void* handle,
                             char* symbol)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("dlsym");

    return 0;
}

static int32_t dlclose_wrapper(wasm_exec_env_t exec_env, void* handle)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("dlclose");

    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(dlopen, "($i)i"),
    REG_NATIVE_FUNC(dlsym, "(*$)i"),
    REG_NATIVE_FUNC(dlclose, "(*)i"),
};

uint32_t getFaasmDynlinkApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
