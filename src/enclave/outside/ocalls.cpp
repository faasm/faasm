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

    int ocallLogWamr(const char* msg) { return printf("%s", msg); }

    // ---------------------------------------
    // WASI Environment calls
    // ---------------------------------------

    int32_t ocallWasiEnvironGet(char* envBuf,
                                int32_t bufLen,
                                int32_t* bytesWritten)
    {
        std::vector<std::string> envVec =
          wasm::getExecutingEnclaveInterface()->getWasmEnvironment().getVars();
        int32_t lengthCount = 0;
        int32_t offset = 0;
        for (std::string& envVar : envVec) {
            lengthCount += envVar.size() + 1;
            if (lengthCount > bufLen) {
                SPDLOG_ERROR("Env. var. length greater than capacity: {} > {}",
                             lengthCount,
                             bufLen);
                throw std::runtime_error(
                  "Env. var. length greater than capacity");
            }
            strncpy(envBuf + offset, envVar.c_str(), envVar.size() + 1);
            offset += envVar.size() + 1;
        }

        *bytesWritten = lengthCount;

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiEnvironSizesGet(int32_t* envCountWasm,
                                     int32_t* envBufferSizeWasm)
    {
        auto wasmEnv =
          wasm::getExecutingEnclaveInterface()->getWasmEnvironment();

        *envCountWasm = wasmEnv.getEnvCount();
        *envBufferSizeWasm = wasmEnv.getEnvBufferSize();

        return __WASI_ESUCCESS;
    }

    // ---------------------------------------
    // Env Filesystem calls
    // ---------------------------------------

    uint32_t doWasiDup(uint32_t fd)
    {
        storage::FileSystem& fs =
          wasm::getExecutingEnclaveInterface()->getFileSystem();
        return fs.dup(fd);
    }

    uint32_t ocallDup(uint32_t fd) { return doWasiDup(fd); }

    // ---------------------------------------
    // WASI Filesystem calls
    // ---------------------------------------

    int32_t doFileStat(uint32_t fd,
                       const std::string& relativePath,
                       uint8_t* wasiFiletype,
                       uint64_t* st_dev,
                       uint64_t* st_ino,
                       uint64_t* st_nlink,
                       uint64_t* st_size,
                       uint64_t* st_atim,
                       uint64_t* st_mtim,
                       uint64_t* st_ctim)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fs = ei->getFileSystem();
        storage::FileDescriptor& fileDesc = fs.getFileDescriptor(fd);
        storage::Stat statNative = fileDesc.stat(relativePath);
        if (statNative.failed) {
            return statNative.wasiErrno;
        }

        *st_dev = statNative.st_dev;
        *st_ino = statNative.st_ino;
        *wasiFiletype = statNative.wasiFiletype;
        *st_nlink = statNative.st_nlink;
        *st_size = statNative.st_size;
        *st_atim = statNative.st_atim;
        *st_mtim = statNative.st_mtim;
        *st_ctim = statNative.st_ctim;

        return __WASI_ESUCCESS;
    }

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

    int32_t ocallWasiFdFilestatGet(int32_t fd,
                                   uint8_t* wasiFiletype,
                                   uint64_t* st_dev,
                                   uint64_t* st_ino,
                                   uint64_t* st_nlink,
                                   uint64_t* st_size,
                                   uint64_t* st_atim,
                                   uint64_t* st_mtim,
                                   uint64_t* st_ctim)
    {
        return doFileStat(fd,
                          "",
                          wasiFiletype,
                          st_dev,
                          st_ino,
                          st_nlink,
                          st_size,
                          st_atim,
                          st_mtim,
                          st_ctim);
    }

    int32_t ocallWasiFdPrestatDirName(int32_t fd, char* path, int32_t pathLen)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fs = ei->getFileSystem();
        if (!fs.fileDescriptorExists(fd)) {
            SPDLOG_ERROR("Fd {} does not exist in filesystem", fd);
            return __WASI_EBADF;
        }

        storage::FileDescriptor& fileDesc = fs.getFileDescriptor(fd);
        std::strncpy(path, fileDesc.getPath().c_str(), pathLen);

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

    int32_t ocallWasiFdRead(int32_t fd,
                            uint8_t* ioVecBases,
                            int32_t ioVecBasesSize,
                            int32_t* ioVecOffsets,
                            int32_t ioVecCount,
                            int32_t* bytesRead)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fileSystem = ei->getFileSystem();
        std::string path = fileSystem.getPathForFd(fd);
        storage::FileDescriptor fileDesc = fileSystem.getFileDescriptor(fd);

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

        // Do the read
        *bytesRead =
          ::readv(fileDesc.getLinuxFd(), ioVecNative.data(), ioVecCount);

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiFdSeek(int32_t fd,
                            int64_t offset,
                            int32_t whence,
                            __wasi_filesize_t* newOffset)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fs = ei->getFileSystem();
        storage::FileDescriptor& fileDesc = fs.getFileDescriptor(fd);
        auto wasiErrno = fileDesc.seek(offset, whence, newOffset);
        return wasiErrno;
    }

    int32_t ocallWasiFdTell(int32_t* returnValue,
                            int32_t fd,
                            int32_t* resOffset)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileDescriptor& fileDesc =
          ei->getFileSystem().getFileDescriptor(fd);

        *resOffset = fileDesc.tell();

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiFdWrite(int32_t fd,
                             uint8_t* ioVecBases,
                             int32_t ioVecBasesSize,
                             int32_t* ioVecOffsets,
                             int32_t ioVecCount,
                             int32_t* bytesWritten)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fileSystem = ei->getFileSystem();
        std::string path = fileSystem.getPathForFd(fd);

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
        storage::FileDescriptor& fileDesc = fileSystem.getFileDescriptor(fd);
        ssize_t n = fileDesc.write(ioVecNative, ioVecCount);
        if (n < 0) {
            SPDLOG_ERROR("writev failed on fd {}: {}",
                         fileDesc.getLinuxFd(),
                         strerror(errno));
        }

        // Write number of bytes to wasm
        *bytesWritten = n;

        // Capture stdout if needed
        conf::FaasmConfig& conf = conf::getFaasmConfig();
        bool isStd = fd <= 2;
        if (isStd && conf.captureStdout == "on") {
            ei->captureStdout(ioVecNative.data(), ioVecCount);
        }

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiPathFilestatGet(int32_t fd,
                                     char* path,
                                     int32_t pathLen,
                                     uint8_t* wasiFiletype,
                                     uint64_t* st_dev,
                                     uint64_t* st_ino,
                                     uint64_t* st_nlink,
                                     uint64_t* st_size,
                                     uint64_t* st_atim,
                                     uint64_t* st_mtim,
                                     uint64_t* st_ctim)
    {
        return doFileStat(fd,
                          std::string(path, pathLen),
                          wasiFiletype,
                          st_dev,
                          st_ino,
                          st_nlink,
                          st_size,
                          st_atim,
                          st_mtim,
                          st_ctim);
    }

    int32_t ocallWasiPathOpen(int32_t fdNative,
                              int32_t lookupFlags,
                              char* path,
                              int32_t pathLen,
                              int32_t openFlags,
                              int64_t rightsBase,
                              int64_t rightsInheriting,
                              int32_t fdFlags,
                              int32_t* fdWasm)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fileSystem = ei->getFileSystem();
        std::string pathStr(path, pathLen);
        *fdWasm = fileSystem.openFileDescriptor(fdNative,
                                                pathStr,
                                                rightsBase,
                                                rightsInheriting,
                                                lookupFlags,
                                                openFlags,
                                                fdFlags);

        if (*fdWasm < 0) {
            return -1 * *fdWasm;
        }

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiPathReadlink(int32_t fd,
                                  char* path,
                                  int32_t pathLen,
                                  char* buf,
                                  int32_t bufLen,
                                  int32_t* resBytesUsed)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fs = ei->getFileSystem();
        storage::FileDescriptor& fileDesc = fs.getFileDescriptor(fd);

        std::string pathStr(path, pathLen);
        *resBytesUsed = fileDesc.readLink(pathStr, buf, bufLen);

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiPathRename(int32_t oldFd,
                                char* oldPath,
                                int32_t oldPathLen,
                                int32_t newFd,
                                char* newPath,
                                int32_t newPathLen)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        std::string newPathStr(newPath, newPathLen);
        std::string oldPathStr(oldPath, oldPathLen);

        storage::FileSystem& fs = ei->getFileSystem();
        storage::FileDescriptor& oldFileDesc = fs.getFileDescriptor(oldFd);
        storage::FileDescriptor& newFileDesc = fs.getFileDescriptor(newFd);

        const std::string& fullNewPath = newFileDesc.absPath(newPathStr);
        bool success = oldFileDesc.rename(fullNewPath, oldPathStr);
        if (!success) {
            return oldFileDesc.getWasiErrno();
        }

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiPathUnlinkFile(int32_t fd, char* path, int32_t pathLen)
    {
        wasm::EnclaveInterface* ei = wasm::getExecutingEnclaveInterface();
        storage::FileSystem& fs = ei->getFileSystem();
        std::string pathStr(path, pathLen);
        storage::FileDescriptor& fileDesc = fs.getFileDescriptor(fd);
        bool success = fileDesc.unlink(pathStr);

        if (!success) {
            return fileDesc.getWasiErrno();
        }

        return __WASI_ESUCCESS;
    }

    // ---------------------------------------
    // Env Filesystem calls
    // ---------------------------------------

    // TODO: remove duplication
    int32_t ocallWasiClockTimeGet(int32_t clockId,
                                  int64_t precision,
                                  int32_t* result)
    {
        timespec ts{};

        // This switch statement is duplicated in src/wavm/timing.cpp the reason
        // being that, even though the constants should be the same, they are
        // defined in runtime-specific headers. Instead of mixing them up, we
        // keep the code duplicated
        int linuxClockId;
        switch (clockId) {
            case __WASI_CLOCK_REALTIME:
                linuxClockId = CLOCK_REALTIME;
                break;
            case __WASI_CLOCK_MONOTONIC:
                linuxClockId = CLOCK_MONOTONIC;
                break;
            case __WASI_CLOCK_PROCESS_CPUTIME_ID:
                linuxClockId = CLOCK_PROCESS_CPUTIME_ID;
                break;
            case __WASI_CLOCK_THREAD_CPUTIME_ID:
                linuxClockId = CLOCK_THREAD_CPUTIME_ID;
                break;
            default:
                SPDLOG_ERROR("Unknown clock ID: {}", clockId);
                throw std::runtime_error("Unknown clock ID");
        }

        int retVal = clock_gettime(linuxClockId, &ts);
        if (retVal < 0) {
            if (EINVAL) {
                return __WASI_EINVAL;
            }
            SPDLOG_ERROR("Unexpected clock error: {}", retVal);
            throw std::runtime_error("Unexpected clock error");
        }

        *result = faabric::util::timespecToNanos(&ts);

        return __WASI_ESUCCESS;
    }
}
