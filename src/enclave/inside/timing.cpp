#include "enclave/inside/EnclaveWasmModule.h"
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

namespace sgx {
uint32_t wasi_clock_time_get(wasm_exec_env_t execEnv,
                             int32_t clockId,
                             int64_t precision,
                             int32_t* result)
{
    SPDLOG_DEBUG_SGX("S - wasi_clock_time_get %i %li", clockId, precision);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(result, sizeof(int32_t));

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiClockTimeGet(
           &returnValue, clockId, precision, result)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return __WASI_ESUCCESS;
}

uint32_t wasi_poll_oneoff(wasm_exec_env_t exec_env,
                          int32_t* a,
                          int64_t* b,
                          int32_t c,
                          int32_t* d)
{
    SPDLOG_DEBUG_SGX("S - wasi_poll_oneoff");
    UNIMPLEMENTED_WASM_INTRINSIC("wasi_poll_oneoff");
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(clock_time_get, "(iI*)i"),
    REG_WASI_NATIVE_FUNC(poll_oneoff, "(**i*)i"),
};

uint32_t getFaasmWasiTimingApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
