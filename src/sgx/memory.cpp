#include <sgx/native.h>

namespace sgx {
static int32_t __sbrk_wrapper(wasm_exec_env_t exec_env, int32_t increment)
{
    int32_t returnValue;
    sgx_status_t sgxReturnValue = ocall_sbrk(&returnValue, increment);

    if ((sgxReturnValue = ocall_sbrk(&returnValue, increment)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__sbrk, "(i)i"),
};

uint32_t getFaasmMemoryApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
