#include <enclave/inside/native.h>

namespace sgx {
static int32_t faasm_read_input_wrapper(wasm_exec_env_t execEnv,
                                        uint8_t* buffer,
                                        unsigned int bufferSize)
{
    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallFaasmReadInput(
           &returnValue, buffer, bufferSize)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
    return returnValue;
}

static void faasm_write_output_wrapper(wasm_exec_env_t execEnv,
                                       char* output,
                                       unsigned int outputSize)
{
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallFaasmWriteOutput(output, outputSize)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
}

static unsigned int faasm_chain_name_wrapper(wasm_exec_env_t execEnv,
                                             const char* name,
                                             const uint8_t* input,
                                             unsigned int inputSize)
{
    sgx_status_t sgxReturnValue;
    unsigned int returnValue;
    if ((sgxReturnValue = ocallFaasmChainName(
           &returnValue, name, input, inputSize)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static unsigned int faasm_chain_ptr_wrapper(wasm_exec_env_t execEnv,
                                            int wasmFuncPtr,
                                            const uint8_t* inputData,
                                            unsigned int inputSize)
{
    // 01/07/2021 - Chain function by pointer is not supported in SGX as it
    // breaks attestation model. Chain by name instead.
    SET_ERROR(FAASM_SGX_WAMR_FUNCTION_NOT_IMPLEMENTED);

    return 1;
}

static unsigned int faasm_await_call_wrapper(wasm_exec_env_t execEnv,
                                             unsigned int callId)
{
    sgx_status_t sgxReturnValue;
    unsigned int returnValue;
    if ((sgxReturnValue = ocallFaasmAwaitCall(&returnValue, callId)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
    return returnValue;
}

static unsigned int faasm_await_call_output_wrapper(wasm_exec_env_t execEnv,
                                                    unsigned int callId,
                                                    char* buffer,
                                                    unsigned int bufferSize)
{
    sgx_status_t sgxReturnValue;
    unsigned int returnValue;
    if ((sgxReturnValue = ocallFaasmAwaitCallOutput(
           &returnValue, callId, buffer, bufferSize)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
    return returnValue;
}

static NativeSymbol ns[] = {
    REG_FAASM_NATIVE_FUNC(faasm_read_input, "($i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_write_output, "($i)"),
    REG_FAASM_NATIVE_FUNC(faasm_chain_name, "($$i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_chain_ptr, "(*$i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_await_call, "(i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_await_call_output, "(i$i)i"),
};

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
