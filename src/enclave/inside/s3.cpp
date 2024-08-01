#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>

namespace sgx {
static int32_t faasm_s3_get_num_buckets_wrapper(wasm_exec_env_t execEnv)
{
    SPDLOG_DEBUG_SGX("S - faasm_s3_get_num_buckets");

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3GetNumBuckets(&returnValue)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static void faasm_s3_list_buckets_wrapper(wasm_exec_env_t execEnv,
                                          int32_t* bucketsBuffer,
                                          int32_t* bucketsBufferLen)
{
    SPDLOG_DEBUG_SGX("faasm_s3_list_buckets");
    auto module = wasm::getExecutingEnclaveWasmModule(execEnv);

    // Do an OCall with two sufficiently large buffers that we are gonna read
    // and use to populate the WASM provided pointers. We use the return
    // value of the OCall to know how many buckets there are
    size_t bufferLen = 2048;
    std::vector<uint8_t> tmpBuffer(bufferLen);
    std::vector<uint8_t> tmpBufferLens(bufferLen);

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3ListBuckets(
           &returnValue, tmpBuffer.data(), tmpBufferLens.data(), bufferLen)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // Sanity check that each pointer-to-array is large enough
    module->validateWasmOffset(module->nativePointerToWasmOffset(bucketsBuffer),
                               returnValue * sizeof(char*));
    module->validateWasmOffset(
      module->nativePointerToWasmOffset(bucketsBufferLen),
      returnValue * sizeof(int32_t));

    // Return value holds the number of different buckets
    size_t bufferOffset = 0;
    for (int i = 0; i < returnValue; i++) {
        // First, copy the size of the buffer
        std::memcpy(&bucketsBufferLen[i],
                    tmpBufferLens.data() + i * sizeof(int32_t),
                    sizeof(int32_t));

        // Second, allocate memory in WASM's heap to copy the buffer name into
        void* nativePtr = nullptr;
        auto wasmOffset =
          module->wasmModuleMalloc(bucketsBufferLen[i], &nativePtr);
        if (wasmOffset == 0 || nativePtr == nullptr) {
            SPDLOG_ERROR_SGX("Error allocating memory in WASM module");
            auto exc = std::runtime_error("Error allocating memory in module!");
            module->doThrowException(exc);
        }

        // Copy the string contents into the newly allocated pointer
        std::memcpy(
          nativePtr, tmpBuffer.data() + bufferOffset, bucketsBufferLen[i]);

        // Store in the i-th entry of the array, a (WASM) pointer to the newly
        // allocated string
        bucketsBuffer[i] = wasmOffset;

        // Lastly, increment the offset in the main buffer
        bufferOffset += bucketsBufferLen[i];
    }
}

static int32_t faasm_s3_get_num_keys_wrapper(wasm_exec_env_t execEnv,
                                             const char* bucketName)
{
    SPDLOG_DEBUG_SGX("S - faasm_s3_get_num_keys (bucket: %s)", bucketName);

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3GetNumKeys(&returnValue, bucketName)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static void faasm_s3_list_keys_wrapper(wasm_exec_env_t execEnv,
                                       char* bucketName,
                                       int32_t* keysBuffer,
                                       int32_t* keysBufferLen)
{
    SPDLOG_DEBUG_SGX("S - faasm_s3_list_keys (bucket: %s)", bucketName);
    auto module = wasm::getExecutingEnclaveWasmModule(execEnv);

    // Do an OCall with two sufficiently large buffers that we are gonna read
    // and use to populate the WASM provided pointers. We use the return
    // value of the OCall to know how many buckets there are
    size_t bufferLen = 2048;
    std::vector<uint8_t> tmpBuffer(bufferLen);
    std::vector<uint8_t> tmpBufferLens(bufferLen);

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3ListKeys(&returnValue,
                                          bucketName,
                                          tmpBuffer.data(),
                                          tmpBufferLens.data(),
                                          bufferLen)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // Sanity check that each pointer-to-array is large enough
    module->validateWasmOffset(module->nativePointerToWasmOffset(keysBuffer),
                               returnValue * sizeof(char*));
    module->validateWasmOffset(module->nativePointerToWasmOffset(keysBufferLen),
                               returnValue * sizeof(int32_t));

    // Return value holds the number of different buckets
    size_t bufferOffset = 0;
    for (int i = 0; i < returnValue; i++) {
        // First, copy the size of the buffer
        std::memcpy(&keysBufferLen[i],
                    tmpBufferLens.data() + i * sizeof(int32_t),
                    sizeof(int32_t));

        // Second, allocate memory in WASM's heap to copy the buffer name into
        void* nativePtr = nullptr;
        auto wasmOffset =
          module->wasmModuleMalloc(keysBufferLen[i], &nativePtr);
        if (wasmOffset == 0 || nativePtr == nullptr) {
            SPDLOG_ERROR_SGX("Error allocating memory in WASM module");
            auto exc = std::runtime_error("Error allocating memory in module!");
            module->doThrowException(exc);
        }

        // Copy the string contents into the newly allocated pointer
        std::memcpy(
          nativePtr, tmpBuffer.data() + bufferOffset, keysBufferLen[i]);

        // Store in the i-th entry of the array, a (WASM) pointer to the newly
        // allocated string
        keysBuffer[i] = wasmOffset;

        // Lastly, increment the offset in the main buffer
        bufferOffset += keysBufferLen[i];
    }
}

static int32_t faasm_s3_add_key_bytes_wrapper(wasm_exec_env_t execEnv,
                                              const char* bucketName,
                                              const char* keyName,
                                              uint8_t* keyBuffer,
                                              int32_t keyBufferLen)
{
    SPDLOG_DEBUG_SGX(
      "faasm_s3_add_key_bytes (bucket: %s, key: %s)", bucketName, keyName);

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3AddKeyBytes(
           &returnValue, bucketName, keyName, keyBuffer, keyBufferLen)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static int32_t faasm_s3_get_key_bytes_wrapper(wasm_exec_env_t execEnv,
                                              const char* bucketName,
                                              const char* keyName,
                                              int32_t* keyBuffer,
                                              int32_t* keyBufferLen)
{
    SPDLOG_DEBUG_SGX(
      "faasm_s3_get_key_bytes (bucket: %s, key: %s)", bucketName, keyName);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Use a temporary, fixed-size, buffer to get the  key bytes. Use the
    // return value to know how many bytes we have got
    size_t bufferLen = 2048;
    std::vector<uint8_t> tmpBuffer(bufferLen);

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3GetKeyBytes(
           &returnValue, bucketName, keyName, tmpBuffer.data(), bufferLen)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    if (returnValue == 0) {
        return 0;
    }

    // Second, allocate memory in WASM's heap to copy the buffer name into
    void* nativePtr = nullptr;
    auto wasmOffset = module->wasmModuleMalloc(returnValue, &nativePtr);

    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR_SGX("Error allocating memory in WASM module");
        auto exc = std::runtime_error("Error allocating memory in module!");
        module->doThrowException(exc);
    }

    // Copy the string contents into the newly allocated pointer
    std::memcpy(nativePtr, tmpBuffer.data(), returnValue);

    // Lastly, populate the given pointers with the new values
    *keyBuffer = wasmOffset;
    *keyBufferLen = returnValue;

    return 0;
}

static NativeSymbol s3Ns[] = {
    REG_FAASM_NATIVE_FUNC(faasm_s3_get_num_buckets, "()i"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_list_buckets, "(**)"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_get_num_keys, "($)i"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_list_keys, "($**)"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_add_key_bytes, "($$*~)i"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_get_key_bytes, "($$**)i"),
};

uint32_t getFaasmS3Api(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = s3Ns;
    return sizeof(s3Ns) / sizeof(NativeSymbol);
}
}
