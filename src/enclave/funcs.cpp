#include <enclave/native.h>

namespace sgx {
static int32_t faasm_read_input_wrapper(wasm_exec_env_t exec_env,
                                        uint8_t* buffer,
                                        unsigned int buffer_size)
{
    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocall_faasm_read_input(
           &returnValue, buffer, buffer_size)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
    return returnValue;
}

static void faasm_write_output_wrapper(wasm_exec_env_t exec_env,
                                       uint8_t* output,
                                       unsigned int output_size)
{
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocall_faasm_write_output(output, output_size)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
}

static unsigned int faasm_chain_name_wrapper(wasm_exec_env_t exec_env,
                                             const char* name,
                                             const uint8_t* input,
                                             unsigned int input_size)
{
    sgx_status_t sgxReturnValue;
    unsigned int returnValue;
    if ((sgxReturnValue = ocall_faasm_chain_name(
           &returnValue, name, input, input_size)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static unsigned int faasm_chain_ptr_wrapper(wasm_exec_env_t exec_env,
                                            int wasmFuncPtr,
                                            const uint8_t* input_data,
                                            unsigned int input_size)
{
    // 01/07/2021 - Chain function by pointer is not supported in SGX as it
    // breaks attestation model. Chain by name instead.
    SET_ERROR(FAASM_SGX_WAMR_FUNCTION_NOT_IMPLEMENTED);

    return 1;
}

static unsigned int faasm_await_call_wrapper(wasm_exec_env_t exec_env,
                                             unsigned int call_id)
{
    sgx_status_t sgxReturnValue;
    unsigned int returnValue;
    if ((sgxReturnValue = ocall_faasm_await_call(&returnValue, call_id)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
    return returnValue;
}

static unsigned int faasm_await_call_output_wrapper(wasm_exec_env_t exec_env,
                                                    unsigned int call_id,
                                                    uint8_t* buffer,
                                                    unsigned int buffer_size)
{
    sgx_status_t sgxReturnValue;
    unsigned int returnValue;
    if ((sgxReturnValue = ocall_faasm_await_call_output(
           &returnValue, call_id, buffer, buffer_size)) != SGX_SUCCESS) {
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
    REG_FAASM_NATIVE_FUNC(faasm_await_call_output, "(i)i"),
};

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
