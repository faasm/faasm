#pragma once

#include <iwasm/include/wasm_export.h>
#include <sgx.h>
#include <sgx_defs.h>

// This file defines the set of functions that can be called from enclave code
// to the outside (untrusted) Faasm runtime. OCalls in SGX terminology.
extern "C"
{
    extern sgx_status_t SGX_CDECL ocallLogError(const char* msg);

// In enclave release mode (i.e NDEBUG set) we disable debug logging, and
// prevent it from doing an ocall (hence the different signature).
#ifdef FAASM_SGX_DEBUG
    void ocallLogDebug(const char* msg) { ; };
#else
    extern sgx_status_t SGX_CDECL ocallLogDebug(const char* msg);
#endif

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

    extern sgx_status_t SGX_CDECL ocallSbrk(int32_t* returnValue,
                                            int32_t increment);
}
