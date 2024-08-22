#include <enclave/common.h>
#include <enclave/outside/EnclaveInterface.h>
#include <enclave/outside/ecalls.h>
#include <enclave/outside/system.h>
#include <faabric/executor/ExecutorContext.h>
#include <wasm/chaining.h>
#include <wasm/faasm.h>
#include <wasm/s3.h>

#include <cstring>

using namespace faabric::executor;

extern "C"
{

    // ---------------------------------------
    // Logging
    // ---------------------------------------

    void ocallLogDebug(const char* msg)
    {
        SPDLOG_DEBUG("[enclave] {}", msg);
    }

    void ocallLogError(const char* msg)
    {
        SPDLOG_ERROR("[enclave] {}", msg);
    }

    int ocallLogWamr(const char* msg)
    {
        return ::printf("%s", msg);
    }

    int ocallFaasmReadInput(uint8_t* buffer, unsigned int bufferSize)
    {
        return wasm::doFaasmReadInput((char*)buffer, bufferSize);
    }

    void ocallFaasmWriteOutput(char* output, unsigned int outputSize)
    {
        ExecutorContext::get()->getMsg().set_outputdata(output, outputSize);
    }

    unsigned int ocallFaasmChainName(const char* name,
                                     const uint8_t* input,
                                     unsigned int inputSize)
    {
        std::vector<uint8_t> _input(input, input + inputSize);
        return wasm::makeChainedCall(std::string(name), 0, nullptr, _input);
    }

    unsigned int ocallFaasmChainPtr(int wasmFuncPtr,
                                    uint8_t* input,
                                    unsigned int inputSize)
    {
        const std::vector<uint8_t> _input(input, input + inputSize);
        return wasm::makeChainedCall(
          ExecutorContext::get()->getMsg().function(),
          wasmFuncPtr,
          nullptr,
          _input);
    }

    unsigned int ocallFaasmAwaitCall(unsigned int callId)
    {
        return wasm::awaitChainedCall(callId);
    }

    unsigned int ocallFaasmAwaitCallOutput(unsigned int callId,
                                           char* buffer,
                                           unsigned int bufferSize)
    {
        faabric::Message result;

        try {
            result = wasm::awaitChainedCallOutput(callId);
        } catch (std::exception& exc) {
            // TODO: how should we handle exceptions thrown in the host?
            SPDLOG_ERROR("Error awating for chained call {}", callId);
        }

        // TODO: should we just ECall data in?
        std::string outputData = result.outputdata();
        if (outputData.size() > bufferSize) {
            SPDLOG_ERROR(
              "Output data is larger than provisioned buffer! ({} > {})",
              outputData.size(),
              bufferSize);

            throw std::runtime_error(
              "Output data larger than provisioned buffer");
        }
        std::memcpy(buffer, outputData.c_str(), outputData.size());

        // In this case the return value is the size of output data so that
        // we can recover it inside the enclave
        return outputData.size();
    }

    int32_t ocallSbrk(int32_t increment)
    {
        SPDLOG_TRACE("S - __sbrk - {}", increment);
        SPDLOG_WARN("SGX-WAMR sbrk does not allocate more memory");
        return 0;
    }

    // ---------------------------------------
    // WASI Filesystem calls
    // ---------------------------------------

    int32_t ocallWasiFdFdstatGet(int32_t wasmFd,
                                 uint8_t* wasiFileType,
                                 uint64_t* rightsBase,
                                 uint64_t* rightsInheriting)
    {
        wasm::EnclaveInterface* enclaveInt =
          wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fileSystem = enclaveInt->getFileSystem();
        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);
        storage::Stat statNative = fileDesc.stat();

        if (statNative.failed) {
            SPDLOG_ERROR("Failed stat: {}", statNative.wasiErrno);
            return statNative.wasiErrno;
        }

        *wasiFileType = statNative.wasiFiletype;
        *rightsBase = fileDesc.getActualRightsBase();
        *rightsInheriting = fileDesc.getActualRightsInheriting();

        return 0;
    }

    int32_t ocallWasiFdWrite(int32_t wasmFd,
                             uint8_t* ioVecBases,
                             int32_t ioVecBasesSize,
                             int32_t* ioVecOffsets,
                             int32_t ioVecCount,
                             int32_t* bytesWritten)
    {
        wasm::EnclaveInterface* enclaveInt =
          wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fileSystem = enclaveInt->getFileSystem();
        std::string path = fileSystem.getPathForFd(wasmFd);

        // Build a ioVec vector from the serialised arguments
        std::vector<::iovec> ioVecNative(ioVecCount, (::iovec){});
        for (int i = 0; i < ioVecCount; i++) {
            ioVecNative[i] = {
                .iov_base = ioVecBases + ioVecOffsets[i],
                .iov_len = i + 1 < ioVecCount
                             ? (size_t)(ioVecOffsets[i + 1] - ioVecOffsets[i])
                             : (size_t)(ioVecBasesSize - ioVecOffsets[i]),
            };
        }

        // Do the write
        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);
        ssize_t nWritten = fileDesc.write(ioVecNative, ioVecCount);
        if (nWritten < 0) {
            SPDLOG_ERROR("writev failed on fd {}: {}",
                         fileDesc.getLinuxFd(),
                         strerror(errno));
        }

        // Write number of bytes to wasm
        *bytesWritten = nWritten;

        // Capture stdout if needed
        conf::FaasmConfig& conf = conf::getFaasmConfig();
        bool isStd = wasmFd <= 2;
        if (isStd && conf.captureStdout == "on") {
            enclaveInt->captureStdout(ioVecNative.data(), ioVecCount);
        }

        return 0;
    }

    // ----- S3 Calls -----

    int32_t ocallS3GetNumBuckets()
    {
        return wasm::doS3GetNumBuckets();
    }

    int32_t ocallS3ListBuckets(uint8_t* buffer,
                               uint8_t* bufferLens,
                               int32_t bufferSize)
    {
        storage::S3Wrapper s3cli;
        auto bucketList = s3cli.listBuckets();

        size_t totalSize = 0;
        for (int i = 0; i < bucketList.size(); i++) {
            if (totalSize > bufferSize) {
                SPDLOG_ERROR(
                  "Exceeded maximum buffer size copying S3 buckets!");
                throw std::runtime_error("Exceeded maximum buffer size");
            }

            int thisBucketSize = bucketList.at(i).size();
            std::memcpy(bufferLens + i * sizeof(int32_t),
                        &thisBucketSize,
                        sizeof(int32_t));
            std::memcpy(
              buffer + totalSize, bucketList.at(i).c_str(), thisBucketSize);

            // Assuming bucket size is always greater than sizeof int
            totalSize += thisBucketSize;
        }

        return bucketList.size();
    }

    int32_t ocallS3GetNumKeys(const char* bucketName)
    {
        return wasm::doS3GetNumKeys(bucketName);
    }

    int32_t ocallS3ListKeys(const char* bucketName,
                            uint8_t* buffer,
                            uint8_t* bufferLens,
                            int32_t bufferSize)
    {
        storage::S3Wrapper s3cli;
        auto keysList = s3cli.listKeys(bucketName);

        // First, calculate the total amount of data to transfer to know if the
        // OCall buffer will be enough or not
        size_t totalSize = 0;
        for (const auto& key : keysList) {
            totalSize += key.size();
        }

        // If we need to use an ECall to transfer data overwrite the provided
        // buffer by a big-enough buffer. Part of it will still be copied as
        // a result of the OCall, but just 1 KB
        bool mustUseECall = totalSize > MAX_OCALL_BUFFER_SIZE;
        if (mustUseECall) {
            buffer = (uint8_t*) faabric::util::malloc(totalSize);
        }

        size_t writtenOffset = 0;
        for (int i = 0; i < keysList.size(); i++) {
            int thisKeySize = keysList.at(i).size();

            std::memcpy(bufferLens + i * sizeof(int32_t),
                        &thisKeySize,
                        sizeof(int32_t));
            std::memcpy(
              buffer + writtenOffset, keysList.at(i).c_str(), thisKeySize);

            writtenOffset += thisKeySize;
        }

        if (mustUseECall) {
            faasm_sgx_status_t returnValue;
            auto enclaveId = wasm::getExecutingEnclaveInterface()->getEnclaveId();
            sgx_status_t sgxReturnValue =
              ecallCopyDataIn(enclaveId, &returnValue, buffer, totalSize);
            sgx::processECallErrors("Error trying to copy data into enclave",
                               sgxReturnValue,
                               returnValue);
        }

        return keysList.size();
    }

    int32_t ocallS3AddKeyBytes(const char* bucketName,
                               const char* keyName,
                               uint8_t* keyBuffer,
                               int32_t keyBufferLen)
    {
        wasm::doS3AddKeyBytes(
          bucketName, keyName, (void*)keyBuffer, keyBufferLen);

        return 0;
    }

    int32_t ocallS3GetKeySize(const char* bucketName, const char* keyName)
    {
        // First, get the actual key bytes from s3
        storage::S3Wrapper s3cli;

        // This call to s3 may throw an exception
        auto data = s3cli.getKeyBytes(bucketName, keyName);

        return data.size();
    }

    int32_t ocallS3GetKeyBytes(const char* bucketName,
                               const char* keyName,
                               uint8_t* buffer,
                               int32_t bufferSize)
    {
        // First, get the actual key bytes from s3
        storage::S3Wrapper s3cli;

        // This call to s3 may throw an exception
        auto data = s3cli.getKeyBytes(bucketName, keyName);

        if (data.size() > MAX_OCALL_BUFFER_SIZE) {
            faasm_sgx_status_t returnValue;
            auto enclaveId = wasm::getExecutingEnclaveInterface()->getEnclaveId();
            sgx_status_t sgxReturnValue =
              ecallCopyDataIn(enclaveId, &returnValue, data.data(), data.size());
            sgx::processECallErrors("Error trying to copy data into enclave",
                               sgxReturnValue,
                               returnValue);

            return 0;
        }

        // Check that we have enough space in the bufer
        if (data.size() > bufferSize) {
            SPDLOG_ERROR(
              "S3 key is larger than provisioned buffer: {} > {} (key: {}/{})",
              data.size(),
              bufferSize,
              bucketName,
              keyName);
            throw std::runtime_error("S3 key is larger than buffer");
        }

        // Copy the data into the buffer
        std::memcpy(buffer, data.data(), data.size());

        return data.size();
    }
}
