#include <faabric/scheduler/ExecutorContext.h>

#include <enclave/outside/EnclaveInterface.h>
#include <wamr/types.h>
#include <wasm/chaining.h>

#include <cstdio>
#include <cstring>

using namespace faabric::scheduler;

extern "C"
{
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

    void ocallFaasmWriteOutput(uint8_t* output, unsigned int outputSize)
    {
        ExecutorContext::get()->getMsg().set_outputdata((void*)output,
                                                        outputSize);
    }

    unsigned int ocallFaasmChainName(const char* name,
                                     const uint8_t* input,
                                     unsigned int inputSize)
    {
        std::vector<uint8_t> _input(input, input + inputSize);
        return wasm::makeChainedCall(std::string(name), 0, nullptr, _input);
    }

    unsigned int ocallFaasmChainPtr(const int wasmFuncPtr,
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
                                           uint8_t* buffer,
                                           unsigned int bufferSize)
    {
        return wasm::awaitChainedCallOutput(callId, buffer, bufferSize);
    }

    // ---------------------------------------
    // Logging
    // ---------------------------------------

    void ocallLogDebug(const char* msg) { SPDLOG_DEBUG("[enclave] {}", msg); }

    void ocallLogError(const char* msg) { SPDLOG_ERROR("[enclave] {}", msg); }

    void ocallLogWamr(const char* msg) { printf("%s", msg); }

    // ---------------------------------------
    // WASI Filesystem calls
    // ---------------------------------------

    int32_t ocallWasiFdFdstatGet(int32_t fd,
                                 uint8_t* wasiFileType,
                                 uint64_t* rightsBase,
                                 uint64_t* rightsInheriting)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fs = ei->getFileSystem();
        storage::FileDescriptor& fileDesc = fs.getFileDescriptor(fd);
        storage::Stat statNative = fileDesc.stat();

        if (statNative.failed) {
            SPDLOG_ERROR("Failed stat: {}", statNative.wasiErrno);
            return statNative.wasiErrno;
        }

        *wasiFileType = statNative.wasiFiletype;
        *rightsBase = fileDesc.getActualRightsBase();
        *rightsInheriting = fileDesc.getActualRightsInheriting();

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiFdPrestatGet(int32_t fd,
                                  uint8_t* prType,
                                  uint32_t* nameLen)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fileSystem = ei->getFileSystem();
        if (!fileSystem.fileDescriptorExists(fd)) {
            return __WASI_EBADF;
        }

        storage::FileDescriptor& fileDesc = fileSystem.getFileDescriptor(fd);

        *prType = fileDesc.wasiPreopenType;
        *nameLen = fileDesc.getPath().size();

        return __WASI_ESUCCESS;
    }

    // TODO: this is not working
    int32_t ocallWasiFdWrite(int32_t fd,
                             uint8_t** ioVecBases,
                             int32_t ioVecBasesSize,
                             size_t* ioVecLens,
                             int32_t ioVecLensSize,
                             int32_t ioVecCountWasm,
                             int32_t* bytesWritten)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fileSystem = ei->getFileSystem();
        std::string path = fileSystem.getPathForFd(fd);

        // Build a ioVec vector from the serialised arguments
        std::vector<::iovec> ioVecNative(ioVecCountWasm, (::iovec){});
        for (int i = 0; i < ioVecCountWasm; i++) {
            ioVecNative[i] = {
                .iov_base = ioVecBases[i],
                .iov_len = ioVecLens[i],
            };
        }

        // Do the write
        storage::FileDescriptor& fileDesc = fileSystem.getFileDescriptor(fd);
        ssize_t n = fileDesc.write(ioVecNative, ioVecCountWasm);
        if (n < 0) {
            SPDLOG_ERROR(
              "writev failed on fd {}: {}", fileDesc.getLinuxFd(), strerror(errno));
        }

        // Write number of bytes to wasm
        *bytesWritten = n;

        // Capture stdout if needed
        conf::FaasmConfig& conf = conf::getFaasmConfig();
        bool isStd = fd <= 2;
        if (isStd && conf.captureStdout == "on") {
            ei->captureStdout(ioVecNative.data(), ioVecCountWasm);
        }

        return __WASI_ESUCCESS;
    }
}
