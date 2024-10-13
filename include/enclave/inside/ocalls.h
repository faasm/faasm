#pragma once

#include <iwasm/include/wasm_export.h>
#include <sgx.h>
#include <sgx_defs.h>

// This file defines the set of functions that can be called from enclave code
// to the outside (untrusted) Faasm runtime. OCalls in SGX terminology.
// TODO: should we delete this file and use the autogenerated one?
extern "C"
{
    extern sgx_status_t SGX_CDECL ocallLogError(const char* msg);

    extern sgx_status_t SGX_CDECL ocallLogDebug(const char* msg);

// In enclave release mode (i.e NDEBUG set) we disable debug logging
#ifdef FAASM_SGX_DEBUG
#define SPDLOG_DEBUG_SGX(...)                                                  \
    {                                                                          \
        const size_t __bufferSize = 512;                                       \
        char __buffer[__bufferSize];                                           \
        snprintf(__buffer, __bufferSize, __VA_ARGS__);                         \
        ocallLogDebug(__buffer);                                               \
    }
#else
#define SPDLOG_DEBUG_SGX(...) ;
#endif

#define SPDLOG_ERROR_SGX(...)                                                  \
    {                                                                          \
        const size_t __bufferSize = 512;                                       \
        char __buffer[__bufferSize];                                           \
        snprintf(__buffer, __bufferSize, __VA_ARGS__);                         \
        ocallLogError(__buffer);                                               \
    }

    extern sgx_status_t SGX_CDECL ocallFaasmReadInput(int* returnValue,
                                                      uint8_t* buffer,
                                                      unsigned int bufferSize);

    extern sgx_status_t SGX_CDECL
    ocallFaasmWriteOutput(char* output, unsigned int outputSize);

    extern sgx_status_t SGX_CDECL ocallFaasmChainName(unsigned int* returnValue,
                                                      const char* name,
                                                      const uint8_t* input,
                                                      unsigned int inputSize);

    extern sgx_status_t SGX_CDECL ocallFaasmChainPtr(unsigned int* returnValue,
                                                     const int wasmFuncPtr,
                                                     const uint8_t* input,
                                                     unsigned int inputSize);

    extern sgx_status_t SGX_CDECL ocallFaasmAwaitCall(unsigned int* returnValue,
                                                      unsigned int callId);

    extern sgx_status_t SGX_CDECL
    ocallFaasmAwaitCallOutput(unsigned int* returnValue,
                              unsigned int callId,
                              char* buffer,
                              unsigned int bufferSize);

    extern sgx_status_t SGX_CDECL ocallSbrk(int32_t* returnValue,
                                            int32_t increment);

    // ----- WASI Filesystem Calls -----

    extern sgx_status_t SGX_CDECL ocallWasiFdDup(int32_t* returnValue,
                                                 int32_t wasmFd);

    extern sgx_status_t SGX_CDECL
    ocallWasiFdFdstatGet(int32_t* returnValue,
                         int32_t wasmFd,
                         uint8_t* wasiFileType,
                         uint64_t* rightsBase,
                         uint64_t* rightsInheriting);

    extern sgx_status_t SGX_CDECL ocallWasiFdFilestatGet(int32_t* returnValue,
                                                         int32_t fd,
                                                         uint8_t* wasiFiletype,
                                                         uint64_t* st_dev,
                                                         uint64_t* st_ino,
                                                         uint64_t* st_nlink,
                                                         uint64_t* st_size,
                                                         uint64_t* st_atim,
                                                         uint64_t* st_mtim,
                                                         uint64_t* st_ctim);

    extern sgx_status_t SGX_CDECL
    ocallWasiFdPrestatDirName(int32_t* returnValue,
                              int32_t wasmFd,
                              char* path,
                              int32_t pathLen);

    extern sgx_status_t SGX_CDECL ocallWasiFdPrestatGet(int32_t* returnValue,
                                                        int32_t wasmFd,
                                                        uint8_t* prType,
                                                        uint32_t* nameLen);

    extern sgx_status_t SGX_CDECL ocallWasiFdRead(int32_t* returnValue,
                                                  int32_t wasmFd,
                                                  uint8_t* ioVecBases,
                                                  int32_t ioVecBasesSize,
                                                  int32_t* ioVecOffsets,
                                                  int32_t ioVecCount,
                                                  int32_t* bytesWritten);

    extern sgx_status_t SGX_CDECL ocallWasiFdReadDir(int32_t* returnValue,
                                                     int32_t wasmFd,
                                                     char* buf,
                                                     uint32_t bufLen,
                                                     int64_t startCookie,
                                                     int32_t* resSizePtr);

    extern sgx_status_t SGX_CDECL ocallWasiFdSeek(int32_t* returnValue,
                                                  int32_t wasmFd,
                                                  int64_t offset,
                                                  int32_t whence,
                                                  uint64_t* newOffset);

    extern sgx_status_t SGX_CDECL ocallWasiFdWrite(int32_t* returnValue,
                                                   int32_t wasmFd,
                                                   uint8_t* ioVecBases,
                                                   int32_t ioVecBasesSize,
                                                   int32_t* ioVecOffsets,
                                                   int32_t ioVecCount,
                                                   int32_t* bytesWritten);

    extern sgx_status_t SGX_CDECL
    ocallWasiPathFilestatGet(int32_t* returnValue,
                             int32_t wasmFd,
                             char* path,
                             int32_t pathLen,
                             uint8_t* wasiFiletype,
                             uint64_t* st_dev,
                             uint64_t* st_ino,
                             uint64_t* st_nlink,
                             uint64_t* st_size,
                             uint64_t* st_atim,
                             uint64_t* st_mtim,
                             uint64_t* st_ctim);

    extern sgx_status_t SGX_CDECL ocallWasiPathOpen(int32_t* returnValue,
                                                    int32_t fdNative,
                                                    int32_t lookupFlags,
                                                    char* path,
                                                    int32_t pathLen,
                                                    int32_t openFlags,
                                                    int64_t rightsBase,
                                                    int64_t rightsInheriting,
                                                    int32_t fdFlags,
                                                    int32_t* fdWasm);

    extern sgx_status_t SGX_CDECL ocallWasiPathReadlink(int32_t* returnValue,
                                                        int32_t wasmFd,
                                                        char* path,
                                                        int32_t pathLen,
                                                        char* buf,
                                                        int32_t bufLen,
                                                        int32_t* resBytesUsed);

    extern sgx_status_t SGX_CDECL ocallWasiPathRename(int32_t* returnValue,
                                                      int32_t oldFd,
                                                      char* oldPath,
                                                      int32_t oldPathLen,
                                                      int32_t newFd,
                                                      char* newPath,
                                                      int32_t newPathLen);

    extern sgx_status_t SGX_CDECL ocallWasiPathUnlinkFile(int32_t* returnValue,
                                                          int32_t wasmFd,
                                                          char* path,
                                                          int32_t pathLen);

    // ----- S3 Calls -----

    extern sgx_status_t SGX_CDECL ocallS3GetNumBuckets(int32_t* returnValue);

    extern sgx_status_t SGX_CDECL ocallS3ListBuckets(int32_t* returnValue,
                                                     uint8_t* buffer,
                                                     uint8_t* bufferLens,
                                                     int32_t bufferSize);

    extern sgx_status_t SGX_CDECL ocallS3GetNumKeys(int32_t* returnValue,
                                                    const char* bucketName,
                                                    const char* prefix,
                                                    int32_t* totalSize,
                                                    bool cache);

    extern sgx_status_t SGX_CDECL ocallS3ListKeys(int32_t* returnValue,
                                                  const char* bucketName,
                                                  const char* prefix);

    extern sgx_status_t SGX_CDECL ocallS3AddKeyBytes(int32_t* returnValue,
                                                     const char* bucketName,
                                                     const char* keyName,
                                                     uint8_t* keyBuffer,
                                                     int32_t keyBufferLen);

    extern sgx_status_t SGX_CDECL ocallS3GetKeySize(int32_t* returnValue,
                                                    const char* bucketName,
                                                    const char* keyName);

    extern sgx_status_t SGX_CDECL ocallS3GetKeyBytes(int32_t* returnValue,
                                                     const char* bucketName,
                                                     const char* keyName,
                                                     uint8_t* buffer,
                                                     int32_t bufferSize);
}
