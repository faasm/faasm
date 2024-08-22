#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>

#include <string>

namespace sgx {
static int32_t faasm_read_input_wrapper(wasm_exec_env_t execEnv,
                                        uint8_t* buffer,
                                        unsigned int bufferSize)
{
    if (bufferSize == 0) {
        SPDLOG_DEBUG_SGX("S - faasm_read_input_size");
    } else {
        SPDLOG_DEBUG_SGX("S - faasm_read_input");
    }

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
    std::string outStr(reinterpret_cast<char*>(output), outputSize);
    SPDLOG_DEBUG_SGX("S - faasm_write_output %s", outStr.c_str());

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

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    SPDLOG_DEBUG_SGX("S - faasm_chain_name %s -> %s",
                     module->getBoundFunction().c_str(),
                     name);

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
    UNIMPLEMENTED_WASM_INTRINSIC("faasm_chain_ptr");
}

static unsigned int faasm_await_call_wrapper(wasm_exec_env_t execEnv,
                                             unsigned int callId)
{
    SPDLOG_DEBUG_SGX("S - faasm_wait_call %i", callId);

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
                                                    int* bufferSize)
{
    SPDLOG_DEBUG_SGX("S - faasm_await_call_output %i", callId);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Get the offset for the buffer pointers so that they are not invalidated
    // after memory growth
    int32_t bufferOffset = module->nativePointerToWasmOffset(buffer);
    int32_t bufferSizeOffset = module->nativePointerToWasmOffset(bufferSize);

    // Use a temporary, fixed-size, buffer for the OCall. If the output data
    // is larger than the buffer, we will report an error. This is to work-
    // around the fact that we can not pass double-pointers across the SGX
    // edge
    std::vector<char> tmpBuf(1024);

    sgx_status_t sgxReturnValue;
    unsigned int returnValue;
    if ((sgxReturnValue = ocallFaasmAwaitCallOutput(
           &returnValue, callId, tmpBuf.data(), tmpBuf.size())) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // We use the return value to let us know how much of the buffer we have
    // written into
    void* nativePtr = nullptr;
    auto wasmOffset = module->wasmModuleMalloc(returnValue, &nativePtr);
    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR_SGX("Error allocating memory in WASM module");
        auto exc = std::runtime_error("Error allocating memory in module!");
        module->doThrowException(exc);
    }
    std::memcpy(nativePtr, tmpBuf.data(), returnValue);

    // Populate the provided pointers
    int32_t* bufferPtr = (int32_t*) module->wasmOffsetToNativePointer(bufferOffset);
    *bufferPtr = wasmOffset;

    int32_t* bufferSizePtr = (int32_t*) module->wasmOffsetToNativePointer(bufferSizeOffset);
    *bufferSizePtr = returnValue;

    return 0;
}

static NativeSymbol funcsNs[] = {
    REG_FAASM_NATIVE_FUNC(faasm_read_input, "($i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_write_output, "($i)"),
    REG_FAASM_NATIVE_FUNC(faasm_chain_name, "($$i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_chain_ptr, "(*$i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_await_call, "(i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_await_call_output, "(i**)i"),
};

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = funcsNs;
    return sizeof(funcsNs) / sizeof(NativeSymbol);
}
}
