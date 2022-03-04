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
    memcpy(((AOTModuleInstance*)execEnv->module_inst)->cur_exception,          \
           _FAASM_SGX_ERROR_PREFIX,                                            \
           sizeof(_FAASM_SGX_ERROR_PREFIX));                                   \
    *((uint32_t*)&((AOTModuleInstance*)execEnv->module_inst)                   \
        ->cur_exception[sizeof(_FAASM_SGX_ERROR_PREFIX)]) = (X);

/*
 * See the WAMR native header file for a detailed explanation on WAMR's native
 * signatures.
 */

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
