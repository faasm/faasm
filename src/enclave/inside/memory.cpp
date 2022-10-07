#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>

namespace sgx {
static int32_t __sbrk_wrapper(wasm_exec_env_t execEnv, int32_t increment)
{
    /*
    int32_t returnValue;
    sgx_status_t sgxReturnValue;

    if ((sgxReturnValue = ocallSbrk(&returnValue, increment)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
    */
    SPDLOG_DEBUG_SGX("S - __sbrk - %i", increment);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    uint32_t oldBrk = module->getCurrentBrk();

    // TODO(csegarragonz): ideally, we'd share shrinkMemory and growMemory
    // across all runtimes
    if (increment == 0) {
        return oldBrk;
    }
    if (increment < 0) {
        module->shrinkMemory(-1 * increment);
        return oldBrk;
    }
    return module->growMemory(increment);
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
