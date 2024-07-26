#include <faabric/util/logging.h>
#include <storage/S3Wrapper.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <wasm_export.h>

namespace wasm {
static int32_t __faasm_s3_get_num_buckets_wrapper(wasm_exec_env_t exec_env)
{
    SPDLOG_DEBUG("S - faasm_s3_get_num_buckets");

    storage::S3Wrapper s3cli;

    return s3cli.listBuckets().size();
}

static void __faasm_s3_list_buckets_wrapper(wasm_exec_env_t exec_env,
                                            int32_t* bucketsBuffer,
                                            int32_t* bucketsBufferLen)
{
    SPDLOG_DEBUG("S - faasm_s3_list_buckets");

    storage::S3Wrapper s3cli;
    auto bucketList = s3cli.listBuckets();

    auto* module = getExecutingWAMRModule();
    for (int i = 0; i < bucketList.size(); i++) {
        // First, populate the len buffer
        auto bucketSize = bucketList.at(i).size();
        bucketsBufferLen[i] = bucketSize;

        // Second, allocate memory in WASM's heap to copy the buffer name into
        void* nativePtr = nullptr;
        auto wasmOffset =
          module->wasmModuleMalloc(bucketList.at(i).size(), &nativePtr);
        if (wasmOffset == 0 || nativePtr == nullptr) {
            SPDLOG_ERROR("Error allocating memory in WASM module");
            auto exc = std::runtime_error("Error allocating memory in module!");
            module->doThrowException(exc);
        }

        // Copy the string contents into the newly allocated pointer
        std::memcpy(
          nativePtr, bucketList.at(i).c_str(), bucketList.at(i).size());

        // Store in the i-th entry of the array, a (WASM) pointer to the newly
        // allocated string
        bucketsBuffer[i] = wasmOffset;
    }
}

static int32_t __faasm_s3_get_num_keys_wrapper(wasm_exec_env_t exec_env,
                                               const char* bucketName)
{
    SPDLOG_DEBUG("S - faasm_s3_get_num_keys (bucket: {})", bucketName);

    storage::S3Wrapper s3cli;

    return s3cli.listKeys(bucketName).size();
}

static void __faasm_s3_list_keys_wrapper(wasm_exec_env_t exec_env,
                                         char* bucketName,
                                         int32_t* keysBuffer,
                                         int32_t* keysBufferLen)
{
    SPDLOG_DEBUG("S - faasm_s3_list_keys (bucket: {})", bucketName);

    storage::S3Wrapper s3cli;
    auto keyList = s3cli.listKeys(bucketName);

    auto* module = getExecutingWAMRModule();
    for (int i = 0; i < keyList.size(); i++) {
        // First, populate the len buffer
        auto keySize = keyList.at(i).size();
        keysBufferLen[i] = keySize;

        // Second, allocate memory in WASM's heap to copy the buffer name into
        void* nativePtr = nullptr;
        auto wasmOffset =
          module->wasmModuleMalloc(keyList.at(i).size(), &nativePtr);
        if (wasmOffset == 0 || nativePtr == nullptr) {
            SPDLOG_ERROR("Error allocating memory in WASM module");
            auto exc = std::runtime_error("Error allocating memory in module!");
            module->doThrowException(exc);
        }

        // Copy the string contents into the newly allocated pointer
        std::memcpy(nativePtr, keyList.at(i).c_str(), keyList.at(i).size());

        // Store in the i-th entry of the array, a (WASM) pointer to the newly
        // allocated string
        keysBuffer[i] = wasmOffset;
    }
}

static int32_t __faasm_s3_add_key_bytes_wrapper(wasm_exec_env_t exec_env,
                                                const char* bucketName,
                                                const char* keyName,
                                                void* keyBuffer,
                                                int32_t keyBufferLen)
{
    storage::S3Wrapper s3cli;
    std::vector<uint8_t> data;
    data.assign((uint8_t*)keyBuffer, (uint8_t*)keyBuffer + keyBufferLen);

    try {
        s3cli.addKeyBytes(bucketName, keyName, data);
    } catch (std::exception& e) {
        auto* module = getExecutingWAMRModule();
        module->doThrowException(e);
    }

    return 0;
}

static int32_t __faasm_s3_get_key_bytes_wrapper(wasm_exec_env_t exec_env,
                                                const char* bucketName,
                                                const char* keyName,
                                                int32_t* keyBuffer,
                                                int32_t* keyBufferLen)
{
    // First, get the actual key bytes from s3
    storage::S3Wrapper s3cli;
    std::vector<uint8_t> data;
    auto* module = getExecutingWAMRModule();

    try {
        data = s3cli.getKeyBytes(bucketName, keyName);
    } catch (std::exception& e) {
        module->doThrowException(e);
    }

    if (data.empty()) {
        return 0;
    }

    // Second, allocate memory in WASM's heap to copy the buffer name into
    void* nativePtr = nullptr;
    auto wasmOffset = module->wasmModuleMalloc(data.size(), &nativePtr);

    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR("Error allocating memory in WASM module");
        auto exc = std::runtime_error("Error allocating memory in module!");
        module->doThrowException(exc);
    }

    // Third, populate the given pointers with the new values
    *keyBuffer = wasmOffset;
    *keyBufferLen = data.size();

    // Copy the string contents into the newly allocated pointer
    std::memcpy(nativePtr, data.data(), data.size());

    return 0;
}

static NativeSymbol s3_ns[] = {
    REG_NATIVE_FUNC(__faasm_s3_get_num_buckets, "()i"),
    REG_NATIVE_FUNC(__faasm_s3_list_buckets, "(**)"),
    REG_NATIVE_FUNC(__faasm_s3_get_num_keys, "($)i"),
    REG_NATIVE_FUNC(__faasm_s3_list_keys, "($**)"),
    REG_NATIVE_FUNC(__faasm_s3_add_key_bytes, "($$*~)i"),
    REG_NATIVE_FUNC(__faasm_s3_get_key_bytes, "($$**)i"),
};

uint32_t getFaasmStateApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = s3_ns;
    return sizeof(s3_ns) / sizeof(NativeSymbol);
}
}
