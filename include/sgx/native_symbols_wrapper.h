#pragma once

#include <iwasm/include/wasm_export.h>
#include <sgx.h>
#include <sgx_defs.h>

// Define symbols that need to be available for functions inside enclaves
extern "C"
{

    // --------------------------------------
    // OCALLS
    // --------------------------------------

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

    // --------------------------------------
    // FAASM HOST INTERFACE
    // --------------------------------------

    static uint64_t faasm_read_state_wrapper(wasm_exec_env_t exec_env,
                                             const char* key,
                                             uint8_t* bufferPtr,
                                             const uint32_t bufferLen);

    static void faasm_read_appended_state_wrapper(wasm_exec_env_t exec_env,
                                                  const char* key,
                                                  uint8_t* bufferPtr,
                                                  const uint32_t bufferLen,
                                                  const uint32_t element_num);

    static void faasm_read_state_offset_wrapper(wasm_exec_env_t exec_env,
                                                const char* key,
                                                const uint64_t totalLen,
                                                const uint64_t offset,
                                                uint8_t* bufferPtr,
                                                const uint32_t bufferLen);

    static void faasm_write_state_wrapper(wasm_exec_env_t exec_env,
                                          const char* key,
                                          const uint8_t* bufferPtr,
                                          const uint32_t bufferLen);

    static void faasm_append_state_wrapper(wasm_exec_env_t exec_env,
                                           const char* key,
                                           const uint8_t* bufferPtr,
                                           const uint32_t bufferLen);

    static void faasm_clear_appended_state_wrapper(wasm_exec_env_t exec_env,
                                                   const char* key);

    static void faasm_write_state_offset_wrapper(wasm_exec_env_t exec_env,
                                                 const char* key,
                                                 const uint64_t totalLen,
                                                 const uint64_t offset,
                                                 const uint8_t* bufferPtr,
                                                 const uint32_t bufferLen);

    static void faasm_flag_state_dirty_wrapper(wasm_exec_env_t exec_env,
                                               const char* key,
                                               const uint64_t totalLen);

    static void faasm_flag_state_offset_dirty_wrapper(wasm_exec_env_t exec_env,
                                                      const char* key,
                                                      const uint64_t totalLen,
                                                      const uint64_t offset,
                                                      const uint64_t len);

    static void faasm_push_state_wrapper(wasm_exec_env_t exec_env,
                                         const char* key);

    static void faasm_push_state_partial_wrapper(wasm_exec_env_t exec_env,
                                                 const char* key);

    static void faasm_push_state_partial_mask_wrapper(wasm_exec_env_t exec_env,
                                                      const char* key,
                                                      const char* mask_key);

    static void faasm_pull_state_wrapper(wasm_exec_env_t exec_env,
                                         const char* key,
                                         const uint64_t state_len);

    static void faasm_lock_state_global_wrapper(wasm_exec_env_t exec_env,
                                                const char* key);

    static void faasm_unlock_state_global_wrapper(wasm_exec_env_t exec_env,
                                                  const char* key);

    static void faasm_lock_state_read_wrapper(wasm_exec_env_t exec_env,
                                              const char* key);

    static void faasm_unlock_state_read_wrapper(wasm_exec_env_t exec_env,
                                                const char* key);

    static void faasm_lock_state_write_wrapper(wasm_exec_env_t exec_env,
                                               const char* key);

    static void faasm_unlock_state_write_wrapper(wasm_exec_env_t exec_env,
                                                 const char* key);

    static int32_t faasm_read_input_wrapper(wasm_exec_env_t exec_env,
                                            uint8_t* buffer,
                                            unsigned int buffer_size);

    static void faasm_write_output_wrapper(wasm_exec_env_t exec_env,
                                           uint8_t* output,
                                           unsigned int output_size);

    static unsigned int faasm_chain_name_wrapper(wasm_exec_env_t exec_env,
                                                 const char* name,
                                                 const uint8_t* input,
                                                 unsigned int input_size);

    static unsigned int faasm_chain_ptr_wrapper(wasm_exec_env_t exec_env,
                                                int wasmFuncPtr,
                                                const uint8_t* input,
                                                unsigned int input_size);

    static unsigned int faasm_await_call_wrapper(wasm_exec_env_t exec_env,
                                                 unsigned int callId);

    static unsigned int faasm_await_call_output_wrapper(
      wasm_exec_env_t exec_env,
      unsigned int callId,
      uint8_t* buffer,
      unsigned int buffer_size);

    // --------------------------------------
    // WHITELISTING
    // --------------------------------------

    void sgx_wamr_function_not_whitelisted_wrapper(wasm_exec_env_t exec_env);

    // -------------------------------------------
    // PTHREADS
    // -------------------------------------------

    static int32_t pthread_mutex_init_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a,
                                              int32_t b);

    static int32_t pthread_mutex_lock_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a);

    static int32_t pthread_mutex_unlock_wrapper(wasm_exec_env_t exec_env,
                                                int32_t a);

    static int32_t pthread_cond_broadcast_wrapper(wasm_exec_env_t exec_env,
                                                  int32_t a);

    static int32_t pthread_mutexattr_init_wrapper(wasm_exec_env_t exec_env,
                                                  int32_t a);

    static int32_t pthread_mutexattr_destroy_wrapper(wasm_exec_env_t exec_env,
                                                     int32_t a);

    static int32_t pthread_equal_wrapper(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int32_t b);

    // ------------------------------
    // MEMORY
    // ------------------------------

    static int32_t __sbrk_wrapper(wasm_exec_env_t exec_env, int32_t increment);

    // --------------------------------------
    // WASI STUBS
    // --------------------------------------

    static int args_get_wrapper(wasm_exec_env_t exec_env, int a, int b);

    static int args_sizes_get_wrapper(wasm_exec_env_t exec_env, int a, int b);

    static int fd_close_wrapper(wasm_exec_env_t exec_env, int a);

    static int fd_seek_wrapper(wasm_exec_env_t exec_env,
                               int a,
                               int64_t b,
                               int c,
                               int d);

    static int fd_write_wrapper(wasm_exec_env_t exec_env,
                                int a,
                                int b,
                                int c,
                                int d);

    static int fd_fdstat_get_wrapper(wasm_exec_env_t exec_env, int a, int b);

    static void proc_exit_wrapper(wasm_exec_env_t exec_env, int returnCode);
}
