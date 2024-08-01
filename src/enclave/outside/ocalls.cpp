#include <faabric/executor/ExecutorContext.h>

#include <enclave/outside/EnclaveInterface.h>
#include <wasm/chaining.h>

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

    int ocallFaasmReadInput(uint8_t* buffer, unsigned int bufferSize)
    {
        faabric::Message* msg = &ExecutorContext::get()->getMsg();

        unsigned long inputLen = msg->inputdata().size();

        if (bufferSize == 0) {
            return (int)inputLen;
        }

        if (inputLen > 0) {
            const std::string& _input = msg->inputdata();
            if (_input.size() > bufferSize) {
                memcpy(buffer, _input.data(), bufferSize);
                return (int)bufferSize;
            }

            memcpy(buffer, _input.data(), _input.size());
            return (int)inputLen;
        }

        return 0;
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
        // FIXME: fix functionality when implementing S3 for SGX
        return wasm::awaitChainedCallOutput(callId).returnvalue();
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
}
