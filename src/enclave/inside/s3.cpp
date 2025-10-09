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
    auto* module = wasm::getExecutingEnclaveWasmModule(execEnv);

    // Get the offset for the buffer pointers so that they are not invalidated
    // after memory growth
    int32_t bucketsBufferOffset =
      module->nativePointerToWasmOffset(bucketsBuffer);
    int32_t bucketsBufferLenOffset =
      module->nativePointerToWasmOffset(bucketsBufferLen);

    // Do an OCall with two sufficiently large buffers that we are gonna read
    // and use to populate the WASM provided pointers. We use the return
    // value of the OCall to know how many buckets there are
    size_t bufferLen = MAX_OCALL_BUFFER_SIZE;
    std::vector<uint8_t> tmpBuffer(bufferLen);
    std::vector<uint8_t> tmpBufferLens(bufferLen);
    assert(module->dataXferPtr == nullptr);
    assert(module->dataXferSize == 0);

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3ListBuckets(
           &returnValue, tmpBuffer.data(), tmpBufferLens.data(), bufferLen)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // Work-out if we have had to use an ECall to transfer data in, or we can
    // use the temporary buffer
    uint8_t* readBuffer;
    bool haveUsedAuxECall =
      module->dataXferSize != 0 && module->dataXferPtr != nullptr;
    if (haveUsedAuxECall) {
        // If we have used the ECall, we need to copy from the heap-allocated
        // data xfer buffer
        readBuffer = module->dataXferPtr;
    } else {
        readBuffer = tmpBuffer.data();
    }

    // Sanity check that each pointer-to-array is large enough
    module->validateWasmOffset(bucketsBufferOffset,
                               returnValue * sizeof(char*));
    module->validateWasmOffset(bucketsBufferLenOffset,
                               returnValue * sizeof(int32_t));

    // Return value holds the number of different buckets
    size_t readOffset = 0;
    for (int i = 0; i < returnValue; i++) {
        int32_t thisBucketLen = *(tmpBufferLens.data() + i * sizeof(int32_t));

        // Allocate memory in WASM's heap to copy the buffer into
        void* nativePtr = nullptr;
        auto wasmOffset = module->wasmModuleMalloc(thisBucketLen, &nativePtr);
        if (wasmOffset == 0 || nativePtr == nullptr) {
            SPDLOG_ERROR_SGX("Error allocating memory in WASM module");
            auto exc = std::runtime_error("Error allocating memory in module!");
            module->doThrowException(exc);
        }

        // Copy the string contents into the newly allocated pointer
        std::memcpy(nativePtr, readBuffer + readOffset, thisBucketLen);

        // Re-convert the WASM offset to a native pointer to prevent pointer
        // invalidations after memory grow operations
        int32_t* bucketsBufferLenPtr =
          (int32_t*)module->wasmOffsetToNativePointer(bucketsBufferLenOffset);
        int32_t* bucketsBufferPtr =
          (int32_t*)module->wasmOffsetToNativePointer(bucketsBufferOffset);

        // Copy the buffer size and buffer length
        bucketsBufferPtr[i] = wasmOffset;
        bucketsBufferLenPtr[i] = thisBucketLen;

        // Lastly, increment the offset in the main buffer
        readOffset += bucketsBufferLen[i];
    }

    if (haveUsedAuxECall) {
        free(module->dataXferPtr);
        module->dataXferPtr = nullptr;
        module->dataXferSize = 0;
    }
}

static int32_t doGetNumKeys(wasm_exec_env_t execEnv,
                            const char* bucketName,
                            const char* prefix,
                            bool cache)
{
    SPDLOG_DEBUG_SGX(
      "S - faasm_s3_get_num_keys (bucket: %s, prefix: %s)", bucketName, prefix);

    int32_t totalKeysSize;
    sgx_status_t sgxReturnValue;
    int32_t numKeys;
    if ((sgxReturnValue = ocallS3GetNumKeys(
           &numKeys, bucketName, prefix, &totalKeysSize, cache)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    int32_t totalKeyLensSize = numKeys * sizeof(int32_t);

    if (cache) {
        auto* module = wasm::getExecutingEnclaveWasmModule(execEnv);

        if (totalKeysSize > MAX_OCALL_BUFFER_SIZE) {
            SPDLOG_DEBUG_SGX("Pre-allocating keys buffer for %s/%s (size: %i)",
                             bucketName,
                             prefix,
                             totalKeysSize);
            module->dataXferPtr = (uint8_t*)malloc(totalKeysSize);
            module->dataXferSize = totalKeysSize;
        }

        if (totalKeyLensSize > MAX_OCALL_BUFFER_SIZE) {
            SPDLOG_DEBUG_SGX(
              "Pre-allocating key lens buffer for %s/%s (size: %i)",
              bucketName,
              prefix,
              totalKeyLensSize);
            module->dataXferAuxPtr = (uint8_t*)malloc(totalKeyLensSize);
            module->dataXferAuxSize = totalKeyLensSize;
        }
    }

    return numKeys;
}

static int32_t faasm_s3_get_num_keys_wrapper(wasm_exec_env_t execEnv,
                                             const char* bucketName)
{
    return doGetNumKeys(execEnv, bucketName, "", false);
}

static int32_t faasm_s3_get_num_keys_with_prefix_wrapper(
  wasm_exec_env_t execEnv,
  const char* bucketName,
  const char* prefix,
  int* totalSize)
{
    return doGetNumKeys(execEnv, bucketName, prefix, true);
}

static void doListKeys(wasm_exec_env_t execEnv,
                       const char* bucketName,
                       const char* prefix,
                       int32_t* keysBuffer,
                       int32_t* keysBufferLen)
{
    SPDLOG_DEBUG_SGX(
      "S - faasm_s3_list_keys (bucket: %s, prefix: %s)", bucketName, prefix);
    auto* module = wasm::getExecutingEnclaveWasmModule(execEnv);

    // Get the offset for the buffer pointers so that they are not invalidated
    // after memory growth
    int32_t keysBufferOffset = module->nativePointerToWasmOffset(keysBuffer);
    int32_t keysBufferLenOffset =
      module->nativePointerToWasmOffset(keysBufferLen);

    sgx_status_t sgxReturnValue;
    int32_t numKeys;
    if ((sgxReturnValue = ocallS3ListKeys(&numKeys, bucketName, prefix)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    if (module->dataXferSize == 0 || module->dataXferPtr == nullptr ||
        module->dataXferAuxSize == 0 || module->dataXferAuxPtr == nullptr) {
        SPDLOG_ERROR_SGX(
          "S3 list keys expects the auxiliary buffers to be populated!");
        auto exc = std::runtime_error(
          "S3 list keys expects the auxiliary buffers to be populated!");
        module->doThrowException(exc);
    }
    uint8_t* readBuffer = module->dataXferPtr;
    uint8_t* readBufferLens = module->dataXferAuxPtr;

    // Sanity check that each pointer-to-array is large enough
    module->validateWasmOffset(keysBufferOffset, numKeys * sizeof(char*));
    module->validateWasmOffset(keysBufferLenOffset, numKeys * sizeof(int32_t));

    // The caller has allocated space for the vector of ints and char*. We now
    // need to heap-allocate space for all the strings (i.e. the pointed-to
    // values in the char* array. We do it in one chunk to prevent issues with
    // memory growths invalidating pointers to WASM memory
    uint8_t* nativePtr = nullptr;
    auto wasmOffset =
      module->wasmModuleMalloc(module->dataXferSize, (void**)&nativePtr);
    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR_SGX(
          "Error allocating memory in WASM module: %s",
          wasm_runtime_get_exception(module->getModuleInstance()));
        auto exc = std::runtime_error("Error allocating memory in module!");
        module->doThrowException(exc);
    }

    // Re-convert the WASM offset to a native pointer to avoid pointer
    // invalidations after memory grow operations
    int32_t* keysBufferLenPtr =
      (int32_t*)module->wasmOffsetToNativePointer(keysBufferLenOffset);
    int32_t* keysBufferPtr =
      (int32_t*)module->wasmOffsetToNativePointer(keysBufferOffset);

    // Assign, to each key, the corresponding pointer to our malloc-ed pool
    size_t readOffset = 0;
    for (int i = 0; i < numKeys; i++) {
        int32_t thisKeyLen = *(readBufferLens + i * sizeof(int32_t));

        // Copy the string payload into WASM memory
        std::memcpy(
          nativePtr + readOffset, readBuffer + readOffset, thisKeyLen);

        // Copy the buffer size and buffer length
        auto thisWasmOffset =
          module->nativePointerToWasmOffset(nativePtr + readOffset);
        keysBufferPtr[i] = thisWasmOffset;
        keysBufferLenPtr[i] = thisKeyLen;

        // Lastly, increment the read offset
        readOffset += keysBufferLenPtr[i];
    }

    free(module->dataXferPtr);
    module->dataXferPtr = nullptr;
    module->dataXferSize = 0;

    free(module->dataXferAuxPtr);
    module->dataXferAuxPtr = nullptr;
    module->dataXferAuxSize = 0;
}

static void faasm_s3_list_keys_wrapper(wasm_exec_env_t execEnv,
                                       const char* bucketName,
                                       int32_t* keysBuffer,
                                       int32_t* keysBufferLen)
{
    doListKeys(execEnv, bucketName, "", keysBuffer, keysBufferLen);
}

static void faasm_s3_list_keys_with_prefix_wrapper(wasm_exec_env_t execEnv,
                                                   const char* bucketName,
                                                   const char* prefix,
                                                   int32_t* keysBuffer,
                                                   int32_t* keysBufferLen)
{
    doListKeys(execEnv, bucketName, prefix, keysBuffer, keysBufferLen);
}

static int32_t faasm_s3_add_key_bytes_wrapper(wasm_exec_env_t execEnv,
                                              const char* bucketName,
                                              const char* keyName,
                                              uint8_t* keyBuffer,
                                              int32_t keyBufferLen,
                                              bool overwrite)
{
    SPDLOG_DEBUG_SGX(
      "faasm_s3_add_key_bytes (bucket: %s, key: %s)", bucketName, keyName);

    sgx_status_t sgxReturnValue;
    int32_t returnValue;
    if ((sgxReturnValue = ocallS3AddKeyBytes(&returnValue,
                                             bucketName,
                                             keyName,
                                             keyBuffer,
                                             keyBufferLen,
                                             overwrite)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static int32_t faasm_s3_get_key_bytes_wrapper(wasm_exec_env_t execEnv,
                                              const char* bucketName,
                                              const char* keyName,
                                              int32_t* keyBuffer,
                                              int32_t* keyBufferLen,
                                              bool tolerateMissing)
{
    // Make a copy to the native stack to avoid pointer invalidations
    std::string bucketNameStr(bucketName);
    std::string keyNameStr(keyName);

    SPDLOG_DEBUG_SGX("faasm_s3_get_key_bytes (bucket: %s, key: %s)",
                     bucketNameStr.c_str(),
                     keyNameStr.c_str());
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Get the offset for the buffer pointers so that they are not invalidated
    // after memory growth
    int32_t keyBufferOffset = module->nativePointerToWasmOffset(keyBuffer);
    int32_t keyBufferLenOffset =
      module->nativePointerToWasmOffset(keyBufferLen);

    // S3 keys may be very large, so we always ask first for the key size, and
    // then heap-allocate the reception buffer
    sgx_status_t sgxReturnValue;
    int32_t keySize;
    if ((sgxReturnValue = ocallS3GetKeySize(&keySize,
                                            bucketNameStr.c_str(),
                                            keyNameStr.c_str(),
                                            tolerateMissing)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // Handle quickly the case where we tolerate a missing key
    if (tolerateMissing && keySize == 0) {
        *keyBufferLen = 0;
        return 0;
    }

    // If the key is larger than what we can fit in the untrusted app's stack,
    // we need to trigger an ECall, and move the memory from the ECall-
    // allocated buffer to our WASM buffer.
    bool mustUseAuxECall = keySize > MAX_OCALL_BUFFER_SIZE;
    assert(module->dataXferPtr == nullptr);
    assert(module->dataXferSize == 0);

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
    if ((sgxReturnValue = ocallS3GetKeyBytes(&copiedBytes,
                                             bucketNameStr.c_str(),
                                             keyNameStr.c_str(),
                                             (uint8_t*)nativePtr,
                                             mustUseAuxECall ? 0 : keySize,
                                             tolerateMissing)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    if (mustUseAuxECall) {
        assert(copiedBytes == 0);

        if (module->dataXferPtr == nullptr ||
            module->dataXferSize != (size_t)keySize) {
            SPDLOG_ERROR_SGX("Copying large buffer in OCall, but have not"
                             " populated auxiliary heap pointer!");
            return 1;
        }

        memcpy(nativePtr, module->dataXferPtr, keySize);

        free(module->dataXferPtr);
        module->dataXferPtr = nullptr;
        module->dataXferSize = 0;
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
    int32_t* keyBufferPtr =
      (int32_t*)module->wasmOffsetToNativePointer(keyBufferOffset);
    *keyBufferPtr = wasmOffset;

    int32_t* keyBufferLenPtr =
      (int32_t*)module->wasmOffsetToNativePointer(keyBufferLenOffset);
    *keyBufferLenPtr = keySize;

    return 0;
}

static NativeSymbol s3Ns[] = {
    REG_FAASM_NATIVE_FUNC(faasm_s3_get_num_buckets, "()i"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_list_buckets, "(**)"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_get_num_keys, "($)i"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_get_num_keys_with_prefix, "($$)i"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_list_keys, "($**)"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_list_keys_with_prefix, "($$**)"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_add_key_bytes, "($$*~i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_s3_get_key_bytes, "($$**i)i"),
};

uint32_t getFaasmS3Api(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = s3Ns;
    return sizeof(s3Ns) / sizeof(NativeSymbol);
}
}
