#include <enclave/common.h>
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
    size_t bufferLen = MAX_OCALL_BUFFER_SIZE;
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
    size_t bufferLen = MAX_OCALL_BUFFER_SIZE;
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

// WARNING: this native function calls wasmModuleMalloc, which in turn calls
// the underlying wasm_runtime_module_malloc. This function call may call heap
// functions inside the module instance and may cause a memory growth. As
// a consequence, native pointers into WASM memory can be invalidated right
// after calling wasm_runtime_module_malloc. As a consequence, make local
// copies of the necessary variables, or use WASM offsets and manually
// translate them right before use. In particular: for the const char* we
// build local copies as strings, and for the buffer and buffer length, which
// are really pointers, we convert the address manually right before use.
static int32_t faasm_s3_get_key_bytes_wrapper(wasm_exec_env_t execEnv,
                                              const char* bucketName,
                                              const char* keyName,
                                              int32_t* keyBuffer,
                                              int32_t* keyBufferLen)
{
    // Make a copy to the native stack to avoid pointer invalidations
    std::string bucketNameStr(bucketName);
    std::string keyNameStr(keyName);

    SPDLOG_DEBUG_SGX(
      "faasm_s3_get_key_bytes (bucket: %s, key: %s)", bucketNameStr.c_str(), keyNameStr.c_str());
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Get the offset for the buffer pointers so that they are not invalidated
    // after memory growth
    int32_t keyBufferOffset = module->nativePointerToWasmOffset(keyBuffer);
    int32_t keyBufferLenOffset = module->nativePointerToWasmOffset(keyBufferLen);

    // S3 keys may be very large, so we always ask first for the key size, and
    // then heap-allocate the reception buffer
    sgx_status_t sgxReturnValue;
    int32_t keySize;
    if ((sgxReturnValue = ocallS3GetKeySize(&keySize, bucketNameStr.c_str(), keyNameStr.c_str())) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // If the key is larger than what we can fit in the untrusted app's stack,
    // we need to trigger an ECall, and move the memory from the ECall-
    // allocated buffer to our WASM buffer.
    bool mustUseAuxEcall = keySize > MAX_OCALL_BUFFER_SIZE;
    if (mustUseAuxEcall) {
        module->dataXferPtr = nullptr;
    }

    void* nativePtr = nullptr;
    auto wasmOffset = module->wasmModuleMalloc(keySize, &nativePtr);
    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR_SGX(
          "Error allocating memory in WASM module: %s",
          wasm_runtime_get_exception(module->getModuleInstance()));
        auto exc = std::runtime_error("Error allocating memory in module!");
        module->doThrowException(exc);
    }

    // Now that we have the buffer, we can give get the key bytes into it
    int copiedBytes;
    if ((sgxReturnValue = ocallS3GetKeyBytes(
           &copiedBytes,
           bucketNameStr.c_str(),
           keyNameStr.c_str(),
           (uint8_t*)nativePtr,
           mustUseAuxEcall ? 0 : keySize)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    if (mustUseAuxEcall) {
        assert(copiedBytes == 0);

        if (module->dataXferPtr == nullptr) {
            SPDLOG_ERROR_SGX("Copying large buffer in OCall, but have not"
                             " populated auxiliary heap pointer!");
            return 1;
        }

        memcpy(nativePtr, module->dataXferPtr, keySize);
        module->dataXferPtr = nullptr;
    } else if (copiedBytes != keySize) {
        SPDLOG_ERROR_SGX(
          "Read different bytes than expected: %i != %i (key: %s/%s)",
          copiedBytes,
          keySize,
          bucketName,
          keyName);
        return 0;
    }

    // Lastly, convert the return variables to pointers and populate them with
    // the new values
    int32_t* keyBufferPtr = (int32_t*) module->wasmOffsetToNativePointer(keyBufferOffset);
    *keyBufferPtr = wasmOffset;

    int32_t* keyBufferLenPtr = (int32_t*) module->wasmOffsetToNativePointer(keyBufferLenOffset);
    *keyBufferLenPtr = keySize;

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
