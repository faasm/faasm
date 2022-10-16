#pragma once

#include <wamr/types.h>

#include <iwasm/include/wasm_export.h>
#include <sgx.h>
#include <sgx_defs.h>

#include <stdio.h>
#include <string>

// For code outside SGX enclaves we use spdlog macros for logging. We provide
// similar macros for logging (only in debug mode) from inside the enclave. Note
// that this logging is very costly as it does an ocall to print the message,
// as a consequence we no-op it at compile time for release builds

#define SPDLOG_DEBUG_SGX(...)                                                  \
    {                                                                          \
        size_t __bufferSize = 512;                                             \
        char __buffer[__bufferSize];                                           \
        snprintf(__buffer, __bufferSize, __VA_ARGS__);                         \
        ocallLogDebug(__buffer);                                               \
    }

#define SPDLOG_ERROR_SGX(...)                                                  \
    {                                                                          \
        size_t __bufferSize = 512;                                             \
        char __buffer[__bufferSize];                                           \
        snprintf(__buffer, __bufferSize, __VA_ARGS__);                         \
        ocallLogError(__buffer);                                               \
    }

// This file defines the set of functions that can be called from enclave code
// to the outside (untrusted) Faasm runtime. OCalls in SGX terminology.
extern "C"
{
    // ----- Logging -----

    extern sgx_status_t SGX_CDECL ocallLogError(const char* msg);

// In enclave release mode (i.e NDEBUG set) we disable debug logging, and
// prevent it from doing an ocall (hence the different signature).
// TODO: isn't this the other way around?
#ifdef FAASM_SGX_DEBUG
    void ocallLogDebug(const char* msg) { ; }
#else
    void ocallLogDebug(const char* msg);
#endif

    // ----- Faasm API -----

    extern sgx_status_t SGX_CDECL ocallFaasmReadInput(int* returnValue,
                                                      uint8_t* buffer,
                                                      unsigned int bufferSize);

    extern sgx_status_t SGX_CDECL
    ocallFaasmWriteOutput(uint8_t* output, unsigned int outputSize);

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
                              uint8_t* buffer,
                              unsigned int bufferSize);

    // ----- WASI Env Calls -----

    extern sgx_status_t SGX_CDECL ocallWasiEnvironGet(int32_t* returnValue,
                                                      char* envBuf,
                                                      int32_t bufLen,
                                                      int32_t* bytesWritten);

    extern sgx_status_t SGX_CDECL
    ocallWasiEnvironSizesGet(int32_t* returnValue,
                             int32_t* envCountWasm,
                             int32_t* envBufferSizeWasm);

    // ----- Env Filesystem Calls -----

    extern sgx_status_t SGX_CDECL ocallDup(uint32_t* returnValue, uint32_t fd);

    // ----- WASI Filesystem Calls -----

    extern sgx_status_t SGX_CDECL
    ocallWasiFdFdstatGet(int32_t* returnValue,
                         int32_t fd,
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
                              int32_t fd,
                              char* path,
                              int32_t pathLen);

    extern sgx_status_t SGX_CDECL ocallWasiFdPrestatGet(int32_t* returnValue,
                                                        int32_t fd,
                                                        uint8_t* prType,
                                                        uint32_t* nameLen);

    extern sgx_status_t SGX_CDECL ocallWasiFdRead(int32_t* returnValue,
                                                  int32_t fd,
                                                  uint8_t** ioVecBases,
                                                  int32_t ioVecBasesSize,
                                                  size_t* ioVecLens,
                                                  int32_t ioVecLensSize,
                                                  int32_t ioVecCountWasm,
                                                  int32_t* bytesWritten);

    extern sgx_status_t SGX_CDECL ocallWasiFdSeek(int32_t* returnValue,
                                                  int32_t fd,
                                                  int64_t offset,
                                                  int32_t whence,
                                                  uint64_t* newOffset);

    extern sgx_status_t SGX_CDECL ocallWasiFdTell(int32_t* returnValue,
                                                  int32_t fd,
                                                  int32_t* resOffset);

    extern sgx_status_t SGX_CDECL ocallWasiFdWrite(int32_t* returnValue,
                                                   int32_t fd,
                                                   uint8_t* ioVecBases,
                                                   int32_t ioVecBasesSize,
                                                   int32_t* ioVecOffsets,
                                                   int32_t ioVecCount,
                                                   int32_t* bytesWritten);

    extern sgx_status_t SGX_CDECL
    ocallWasiPathFilestatGet(int32_t* returnValue,
                             int32_t fd,
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
                                                        int32_t fd,
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
                                                          int32_t fd,
                                                          char* path,
                                                          int32_t pathLen);

    // ---- WASI Timing OCalls ----

    extern sgx_status_t SGX_CDECL ocallWasiClockTimeGet(int32_t* returnValue,
                                                        int32_t clockId,
                                                        int64_t precision,
                                                        int32_t* result);
}
