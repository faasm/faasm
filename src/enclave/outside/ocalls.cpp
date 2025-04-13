#include <enclave/common.h>
#include <enclave/outside/EnclaveInterface.h>
#include <enclave/outside/attestation/AttestationServiceClient.h>
#include <enclave/outside/attestation/attestation.h>
#include <enclave/outside/ecalls.h>
#include <enclave/outside/system.h>
#include <faabric/executor/ExecutorContext.h>
#include <wasm/chaining.h>
#include <wasm/faasm.h>
#include <wasm/s3.h>

#include <cstring>
#include <optional>

#ifdef FAASM_SGX_HARDWARE_MODE
#include <sgx_dcap_ql_wrapper.h>
#include <sgx_ql_lib_common.h>
#endif

// TODO: cannot seem to include the WAMR file with the WASI types, as it seems
// to clash with some SGX definitions
typedef uint64_t __wasi_filesize_t;
#define __WASI_DIRCOOKIE_START (0)

#define __WASI_EBADF (8)
#define __WASI_EINVAL (28)
#define __WASI_ESUCCESS (0)

using namespace faabric::executor;

// Cache to re-use data between successive OCall invocations
std::optional<std::vector<std::string>> s3ListKeysCache;

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

    int32_t doFileStat(uint32_t wasmFd,
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
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);
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

    int32_t ocallWasiFdDup(int32_t wasmFd)
    {
        auto& fileSystem =
          wasm::getExecutingEnclaveInterface()->getFileSystem();

        return fileSystem.dup(wasmFd);
    }

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

    int32_t ocallWasiFdFilestatGet(int32_t wasmFd,
                                   uint8_t* wasiFiletype,
                                   uint64_t* st_dev,
                                   uint64_t* st_ino,
                                   uint64_t* st_nlink,
                                   uint64_t* st_size,
                                   uint64_t* st_atim,
                                   uint64_t* st_mtim,
                                   uint64_t* st_ctim)
    {
        return doFileStat(wasmFd,
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

    int32_t ocallWasiFdPrestatDirName(int32_t wasmFd,
                                      char* path,
                                      int32_t pathLen)
    {
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        if (!fileSystem.fileDescriptorExists(wasmFd)) {
            SPDLOG_ERROR("Fd {} does not exist in filesystem", wasmFd);
            return __WASI_EBADF;
        }

        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);
        std::strncpy(path, fileDesc.getPath().c_str(), pathLen);

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiFdPrestatGet(int32_t wasmFd,
                                  uint8_t* prType,
                                  uint32_t* nameLen)
    {
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        if (!fileSystem.fileDescriptorExists(wasmFd)) {
            return __WASI_EBADF;
        }

        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);

        *prType = fileDesc.wasiPreopenType;
        *nameLen = fileDesc.getPath().size();

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiFdRead(int32_t wasmFd,
                            uint8_t* ioVecBases,
                            int32_t ioVecBasesSize,
                            int32_t* ioVecOffsets,
                            int32_t ioVecCount,
                            int32_t* bytesRead)
    {
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        std::string path = fileSystem.getPathForFd(wasmFd);
        storage::FileDescriptor fileDesc = fileSystem.getFileDescriptor(wasmFd);

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

    int32_t ocallWasiFdReadDir(int32_t wasmFd,
                               char* buf,
                               int32_t bufLen,
                               int64_t startCookie,
                               int32_t* resSizePtr)
    {
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        std::string path = fileSystem.getPathForFd(wasmFd);
        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);

        bool isStartCookie = startCookie == __WASI_DIRCOOKIE_START;
        if (fileDesc.iterStarted() && isStartCookie) {
            // Return invalid if we've already started the iterator but also get
            // the start cookie
            return __WASI_EINVAL;
        }
        if (!fileDesc.iterStarted() && !isStartCookie) {
            throw std::runtime_error(
              "No directory iterator exists, and this is not the start cookie");
        }

        size_t bytesCopied =
          fileDesc.copyDirentsToWasiBuffer((uint8_t*)buf, bufLen);
        *resSizePtr = bytesCopied;

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiFdSeek(int32_t wasmFd,
                            int64_t offset,
                            int32_t whence,
                            __wasi_filesize_t* newOffset)
    {
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        auto& fileDesc = fileSystem.getFileDescriptor(wasmFd);
        auto wasiErrno = fileDesc.seek(offset, whence, newOffset);
        return wasiErrno;
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

    int32_t ocallWasiPathFilestatGet(int32_t wasmFd,
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
        return doFileStat(wasmFd,
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
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
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

    int32_t ocallWasiPathReadlink(int32_t wasmFd,
                                  char* path,
                                  int32_t pathLen,
                                  char* buf,
                                  int32_t bufLen,
                                  int32_t* resBytesUsed)
    {
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);

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
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        std::string newPathStr(newPath, newPathLen);
        std::string oldPathStr(oldPath, oldPathLen);

        auto& fileSystem = enclaveInt->getFileSystem();
        storage::FileDescriptor& oldFileDesc =
          fileSystem.getFileDescriptor(oldFd);
        storage::FileDescriptor& newFileDesc =
          fileSystem.getFileDescriptor(newFd);

        const std::string& fullNewPath = newFileDesc.absPath(newPathStr);
        bool success = oldFileDesc.rename(fullNewPath, oldPathStr);
        if (!success) {
            return oldFileDesc.getWasiErrno();
        }

        return __WASI_ESUCCESS;
    }

    int32_t ocallWasiPathUnlinkFile(int32_t wasmFd, char* path, int32_t pathLen)
    {
        auto* enclaveInt = wasm::getExecutingEnclaveInterface();
        auto& fileSystem = enclaveInt->getFileSystem();
        std::string pathStr(path, pathLen);
        storage::FileDescriptor& fileDesc =
          fileSystem.getFileDescriptor(wasmFd);
        bool success = fileDesc.unlink(pathStr);

        if (!success) {
            return fileDesc.getWasiErrno();
        }

        return __WASI_ESUCCESS;
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

    int32_t ocallS3GetNumKeys(const char* bucketName,
                              const char* prefix,
                              int32_t* totalSize,
                              bool cache)
    {
        storage::S3Wrapper s3cli;
        auto keysList = s3cli.listKeys(bucketName, prefix);

        if (cache) {
            s3ListKeysCache = keysList;
        }

        // Pre-calculate the total size of the keys so that the caller can
        // malloc buffers up-front
        int32_t totalKeysSize = 0;
        for (const auto& key : keysList) {
            totalKeysSize += key.size();
        }
        *totalSize = totalKeysSize;

        return wasm::doS3GetNumKeys(bucketName, prefix);
    }

    int32_t ocallS3ListKeys(const char* bucketName, const char* prefix)
    {
        std::vector<std::string> keysList;

        if (s3ListKeysCache.has_value()) {
            SPDLOG_DEBUG("Using cached key list for {}/{}", bucketName, prefix);
            keysList = *s3ListKeysCache;
            s3ListKeysCache.reset();
        } else {
            storage::S3Wrapper s3cli;
            keysList = s3cli.listKeys(bucketName, prefix);
        }

        // First, calculate the total amount of data to transfer. For
        // simplicity, we always use an ECall to transfer the data in
        size_t totalSize = 0;
        size_t totalLensSize = 0;
        for (const auto& key : keysList) {
            totalSize += key.size();
            totalLensSize += sizeof(int32_t);
        }
        std::vector<uint8_t> auxBuffer = std::vector<uint8_t>(totalSize);
        std::vector<uint8_t> auxLensBuffer =
          std::vector<uint8_t>(totalLensSize);

        // Serialise keys into buffer to transfer via ECall
        size_t writtenOffset = 0;
        for (int i = 0; i < keysList.size(); i++) {
            int thisKeySize = keysList.at(i).size();

            std::memcpy(auxLensBuffer.data() + i * sizeof(int32_t),
                        &thisKeySize,
                        sizeof(int32_t));
            std::memcpy(auxBuffer.data() + writtenOffset,
                        keysList.at(i).c_str(),
                        thisKeySize);

            writtenOffset += thisKeySize;
        }

        // Perform the ECall
        faasm_sgx_status_t returnValue;
        auto enclaveId = wasm::getExecutingEnclaveInterface()->getEnclaveId();
        sgx_status_t sgxReturnValue = ecallCopyDataIn(enclaveId,
                                                      &returnValue,
                                                      auxBuffer.data(),
                                                      auxBuffer.size(),
                                                      auxLensBuffer.data(),
                                                      auxLensBuffer.size());
        sgx::processECallErrors("Error trying to copy data into enclave",
                                sgxReturnValue,
                                returnValue);

        return keysList.size();
    }

    int32_t ocallS3AddKeyBytes(const char* bucketName,
                               const char* keyName,
                               uint8_t* keyBuffer,
                               int32_t keyBufferLen,
                               bool overwrite)
    {
        wasm::doS3AddKeyBytes(
          bucketName, keyName, (void*)keyBuffer, keyBufferLen, overwrite);

        return 0;
    }

    int32_t ocallS3GetKeySize(const char* bucketName,
                              const char* keyName,
                              bool tolerateMissing)
    {
        // First, get the actual key bytes from s3
        storage::S3Wrapper s3cli;

        // This call to s3 may throw an exception
        auto data = s3cli.getKeyBytes(bucketName, keyName, tolerateMissing);

        return data.size();
    }

    int32_t ocallS3GetKeyBytes(const char* bucketName,
                               const char* keyName,
                               uint8_t* buffer,
                               int32_t bufferSize,
                               bool tolerateMissing)
    {
        // First, get the actual key bytes from s3
        storage::S3Wrapper s3cli;

        // This call to s3 may throw an exception
        auto data = s3cli.getKeyBytes(bucketName, keyName, tolerateMissing);

        if (data.size() > MAX_OCALL_BUFFER_SIZE) {
            faasm_sgx_status_t returnValue;
            auto enclaveId =
              wasm::getExecutingEnclaveInterface()->getEnclaveId();
            sgx_status_t sgxReturnValue = ecallCopyDataIn(
              enclaveId, &returnValue, data.data(), data.size(), nullptr, 0);
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

    // ----- Attestation Calls -----

    int32_t ocallAttGetQETargetInfo(void* buffer, int32_t bufferSize)
    {
#ifdef FAASM_SGX_HARDWARE_MODE
        sgx_target_info_t targetInfo = sgx::getQuotingEnclaveTargetInfo();

        // Copy into enclave-provided buffer
        assert(bufferSize == sizeof(targetInfo));
        std::memcpy(buffer, &targetInfo, bufferSize);
#endif

        return 0;
    }

    int32_t ocallAttValidateQuote(sgx_report_t report, int32_t* jwtResponseSize)
    {
#ifdef FAASM_SGX_HARDWARE_MODE
        // First, generate quote
        auto quoteBuffer = sgx::getQuoteFromReport(report);

        // Now, validate it with the attestation service in Azure
        sgx::AttestationServiceClient aaClient(
          conf::getFaasmConfig().attestationServiceUrl);
        std::string jwtResponse = aaClient.attestEnclave(quoteBuffer, report);

        // JWT response contains the actual JWT encrypted (JWT) as well ass
        // the public key used in the key derivation algorithm on the server
        // side
        auto [jwe, pubKey] = aaClient.getTokenFromJwtResponse(jwtResponse);

        // We are careful here: we copy both strings concatenated, but as
        // a result we return only the size of the first one, making it
        // possible to deserialize on the other end
        std::string jwtCombined = jwe + pubKey;

        faasm_sgx_status_t returnValue;
        auto enclaveId = wasm::getExecutingEnclaveInterface()->getEnclaveId();
        sgx_status_t sgxReturnValue =
          ecallCopyDataIn(enclaveId,
                          &returnValue,
                          (uint8_t*)jwtCombined.c_str(),
                          jwtCombined.size(),
                          nullptr,
                          0);
        sgx::processECallErrors("Error trying to copy data into enclave",
                                sgxReturnValue,
                                returnValue);

        *jwtResponseSize = jwe.size();
#endif

        return 0;
    }
}
