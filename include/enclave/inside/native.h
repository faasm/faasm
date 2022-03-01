#pragma once

#include <enclave/error.h>
#include <enclave/inside/ocalls.h>

#include <iwasm/aot/aot_runtime.h>
#include <iwasm/common/wasm_exec_env.h>
#include <wasm_export.h>
#include <wasm_native.h>

#define REG_NATIVE_FUNC(func_name, signature)                                  \
    {                                                                          \
#func_name, (void*)func_name##_wrapper, signature, nullptr             \
    }

#define REG_FAASM_NATIVE_FUNC(func_name, signature)                            \
    {                                                                          \
        "__" #func_name, (void*)func_name##_wrapper, signature, nullptr        \
    }

#define REG_WASI_NATIVE_FUNC(func_name, signature)                             \
    {                                                                          \
#func_name, (void*)wasi_##func_name, signature, nullptr                \
    }

#define SET_ERROR(X)                                                           \
    memcpy(((AOTModuleInstance*)exec_env->module_inst)->cur_exception,         \
           _FAASM_SGX_ERROR_PREFIX,                                            \
           sizeof(_FAASM_SGX_ERROR_PREFIX));                                   \
    *((uint32_t*)&((AOTModuleInstance*)exec_env->module_inst)                  \
        ->cur_exception[sizeof(_FAASM_SGX_ERROR_PREFIX)]) = (X);

// ---------- WAMR Symbol Registration ----------
// The function signature to register native symbols is made up of a parenthesis
// with the type of each input argument, and the type of the return value
// immediately after. E.g: (ii)i # two int32 input arguments, returns an int32
// The different supported types are:
// - i/I,f/F: int32/64 and float32/64
// - *: the parameter is an offset in WASM memory. **Important** if we use `*`
//      instead of `i` the runtime automatically translates the address and does
//      bound checks. Otherwise, we need to translate the address ourselves.
// - ~: following a `*`, a `~` means that the parameter is the size of the
//      preceeding pointer (indicated by a `*`). The runtime does the
//      corresponding bound and address checks.
// - $: string parameter also with runtime checks.
//
// Link to WAMR docs:
// https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md#buffer-address-conversion-and-boundary-check
// ----------------------------------------------

namespace sgx {
void initialiseSGXWAMRNatives();

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmMemoryApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmPthreadApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmStateApi(NativeSymbol** nativeSymbols);

// ---------- WASI symbols ----------

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmWasiFilesystemApi(NativeSymbol** nativeSymbols);
}
