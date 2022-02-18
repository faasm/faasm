#pragma once

#include <iwasm/include/wasm_export.h>
#include <sgx.h>
#include <sgx_defs.h>

// This file defines the set of functions that can be called from enclave code
// to the outside (untrusted) Faasm runtime. OCalls in SGX terminology.
extern "C"
{
    extern sgx_status_t SGX_CDECL ocall_faasm_log_error(const char* msg);

    extern sgx_status_t SGX_CDECL ocall_faasm_log_debug(const char* msg);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_read_state(uint64_t* returnValue,
                           const char* key,
                           uint8_t* buffer,
                           const uint32_t bufferLen);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_read_appended_state(const char* key,
                                    uint8_t* bufferPtr,
                                    const uint32_t bufferLen,
                                    const uint32_t element_num);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_read_state_offset(const char* key,
                                  const uint64_t totalLen,
                                  const uint64_t offset,
                                  uint8_t* bufferPtr,
                                  const uint32_t bufferLen);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_write_state(const char* key,
                            const uint8_t* bufferPtr,
                            const uint32_t bufferLen);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_append_state(const char* key,
                             const uint8_t* bufferPtr,
                             const uint32_t bufferLen);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_clear_appended_state(const char* key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_write_state_offset(const char* key,
                                   const uint64_t totalLen,
                                   const uint64_t offset,
                                   const uint8_t* bufferPtr,
                                   const uint32_t bufferLen);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_flag_state_dirty(const char* key, const uint64_t totalLen);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_flag_state_offset_dirty(const char* key,
                                        const uint64_t totalLen,
                                        const uint64_t offset,
                                        const uint64_t len);

    extern sgx_status_t SGX_CDECL ocall_faasm_push_state(const char* key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_push_state_partial(const char* key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_push_state_partial_mask(const char* key, const char* mask_key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_pull_state(const char* key, const uint64_t state_len);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_lock_state_global(const char* key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_unlock_state_global(const char* key);

    extern sgx_status_t SGX_CDECL ocall_faasm_lock_state_read(const char* key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_unlock_state_read(const char* key);

    extern sgx_status_t SGX_CDECL ocall_faasm_lock_state_write(const char* key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_unlock_state_write(const char* key);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_read_input(int* returnValue,
                           uint8_t* buffer,
                           unsigned int buffer_size);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_write_output(uint8_t* output, unsigned int output_size);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_chain_name(unsigned int* returnValue,
                           const char* name,
                           const uint8_t* input,
                           unsigned int input_size);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_chain_ptr(unsigned int* returnValue,
                          const int wasmFuncPtr,
                          const uint8_t* input,
                          unsigned int input_size);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_await_call(unsigned int* returnValue, unsigned int callId);

    extern sgx_status_t SGX_CDECL
    ocall_faasm_await_call_output(unsigned int* returnValue,
                                  unsigned int callId,
                                  uint8_t* buffer,
                                  unsigned int buffer_size);

    extern sgx_status_t SGX_CDECL ocall_sbrk(int32_t* returnValue,
                                             int32_t increment);
}
