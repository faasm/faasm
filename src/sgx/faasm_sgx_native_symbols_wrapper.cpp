#include <sgx/faasm_sgx_enclave_types.h>
#include <sgx/faasm_sgx_error.h>
#include <sgx/faasm_sgx_native_symbols_wrapper.h>

#include <iwasm/aot/aot_runtime.h>
#include <iwasm/common/wasm_exec_env.h>

#if (FAASM_SGX_ATTESTATION)
extern "C"
{
    extern _faasm_sgx_tcs_t** faasm_sgx_tcs;
    extern __thread uint32_t tls_thread_id;
}
#endif

// SET_ERROR definition
#define SET_ERROR(X)                                                           \
    memcpy(((AOTModuleInstance*)exec_env->module_inst)->cur_exception,         \
           _FAASM_SGX_ERROR_PREFIX,                                            \
           sizeof(_FAASM_SGX_ERROR_PREFIX));                                   \
    *((uint32_t*)&((AOTModuleInstance*)exec_env->module_inst)                  \
        ->cur_exception[sizeof(_FAASM_SGX_ERROR_PREFIX)]) = (X);

#define NATIVE_FUNC(funcName, funcSig)                                         \
    {                                                                          \
        "__" #funcName, (void*)funcName##_wrapper, funcSig, 0x0                \
    }

#define PTHREAD_NATIVE_FUNC(funcName, funcSig)                                 \
    {                                                                          \
#funcName, (void*)funcName##_wrapper, funcSig, 0x0                     \
    }

#define MEMORY_NATIVE_FUNC(funcName, funcSig)                                  \
    {                                                                          \
#funcName, (void*)funcName##_wrapper, funcSig, 0x0                     \
    }

#define WASI_NATIVE_FUNC(funcName, funcSig)                                    \
    {                                                                          \
#funcName, (void*)funcName##_wrapper, funcSig, 0x0                     \
    }

extern "C"
{

    // ------------------------------------
    // FUNCTIONS
    // ------------------------------------

    static int32_t faasm_read_input_wrapper(wasm_exec_env_t exec_env,
                                            uint8_t* buffer,
                                            unsigned int buffer_size)
    {
        int32_t returnValue;
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_read_input(
               &returnValue, buffer, buffer_size)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
        return returnValue;
    }

    static void faasm_write_output_wrapper(wasm_exec_env_t exec_env,
                                           uint8_t* output,
                                           unsigned int output_size)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_write_output(output, output_size)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    // --------------------------------------
    // STATE
    // --------------------------------------

    static uint64_t faasm_read_state_wrapper(wasm_exec_env_t exec_env,
                                             const char* key,
                                             uint8_t* buffer_ptr,
                                             const uint32_t buffer_len)
    {
        sgx_status_t sgxReturnValue;
        uint64_t returnValue;
        if ((sgxReturnValue = ocall_faasm_read_state(
               &returnValue, key, buffer_ptr, buffer_len)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
        return returnValue;
    }

    static void faasm_read_appended_state_wrapper(wasm_exec_env_t exec_env,
                                                  const char* key,
                                                  uint8_t* buffer_ptr,
                                                  const uint32_t buffer_len,
                                                  const uint32_t element_num)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_read_appended_state(
               key, buffer_ptr, buffer_len, element_num)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_read_state_offset_wrapper(wasm_exec_env_t exec_env,
                                                const char* key,
                                                const uint64_t total_len,
                                                const uint64_t offset,
                                                uint8_t* buffer_ptr,
                                                const uint32_t buffer_len)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_read_state_offset(
               key, total_len, offset, buffer_ptr, buffer_len)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_write_state_wrapper(wasm_exec_env_t exec_env,
                                          const char* key,
                                          const uint8_t* buffer_ptr,
                                          const uint32_t buffer_len)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_write_state(
               key, buffer_ptr, buffer_len)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_append_state_wrapper(wasm_exec_env_t exec_env,
                                           const char* key,
                                           const uint8_t* buffer_ptr,
                                           const uint32_t buffer_len)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_append_state(
               key, buffer_ptr, buffer_len)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_clear_appended_state_wrapper(wasm_exec_env_t exec_env,
                                                   const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_clear_appended_state(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_write_state_offset_wrapper(wasm_exec_env_t exec_env,
                                                 const char* key,
                                                 const uint64_t total_len,
                                                 const uint64_t offset,
                                                 const uint8_t* buffer_ptr,
                                                 const uint32_t buffer_len)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_write_state_offset(
               key, total_len, offset, buffer_ptr, buffer_len)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_flag_state_dirty_wrapper(wasm_exec_env_t exec_env,
                                               const char* key,
                                               const uint64_t total_len)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_flag_state_dirty(key, total_len)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_flag_state_offset_dirty_wrapper(wasm_exec_env_t exec_env,
                                                      const char* key,
                                                      const uint64_t total_len,
                                                      const uint64_t offset,
                                                      const uint64_t len)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_flag_state_offset_dirty(
               key, total_len, offset, len)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_push_state_wrapper(wasm_exec_env_t exec_env,
                                         const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_push_state(key)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_push_state_partial_wrapper(wasm_exec_env_t exec_env,
                                                 const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_push_state_partial(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_push_state_partial_mask_wrapper(wasm_exec_env_t exec_env,
                                                      const char* key,
                                                      const char* mask_key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_push_state_partial_mask(
               key, mask_key)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_pull_state_wrapper(wasm_exec_env_t exec_env,
                                         const char* key,
                                         const uint64_t state_len)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_pull_state(key, state_len)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_lock_state_global_wrapper(wasm_exec_env_t exec_env,
                                                const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_lock_state_global(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_unlock_state_global_wrapper(wasm_exec_env_t exec_env,
                                                  const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_unlock_state_global(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_lock_state_read_wrapper(wasm_exec_env_t exec_env,
                                              const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_lock_state_read(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_unlock_state_read_wrapper(wasm_exec_env_t exec_env,
                                                const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_unlock_state_read(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_lock_state_write_wrapper(wasm_exec_env_t exec_env,
                                               const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_lock_state_write(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    static void faasm_unlock_state_write_wrapper(wasm_exec_env_t exec_env,
                                                 const char* key)
    {
        sgx_status_t sgxReturnValue;
        if ((sgxReturnValue = ocall_faasm_unlock_state_write(key)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
    }

    // ------------------------------
    // CHAINING
    // ------------------------------

    static unsigned int faasm_chain_name_wrapper(wasm_exec_env_t exec_env,
                                                 const char* name,
                                                 const uint8_t* input,
                                                 unsigned int input_size)
    {
        sgx_status_t sgxReturnValue;
        unsigned int returnValue;
        if ((sgxReturnValue = ocall_faasm_chain_name(
               &returnValue, name, input, input_size)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }

        return returnValue;
    }

    static unsigned int faasm_chain_ptr_wrapper(wasm_exec_env_t exec_env,
                                                int wasmFuncPtr,
                                                const uint8_t* input_data,
                                                unsigned int input_size)
    {
        // 01/07/2021 - Chain function by pointer is not supported in SGX as it
        // breaks attestation model. Chain by name instead.
        SET_ERROR(FAASM_SGX_WAMR_FUNCTION_NOT_IMPLEMENTED);

        return 1;
    }

    static unsigned int faasm_await_call_wrapper(wasm_exec_env_t exec_env,
                                                 unsigned int call_id)
    {
        sgx_status_t sgxReturnValue;
        unsigned int returnValue;
        if ((sgxReturnValue = ocall_faasm_await_call(&returnValue, call_id)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
        return returnValue;
    }

    static unsigned int faasm_await_call_output_wrapper(
      wasm_exec_env_t exec_env,
      unsigned int call_id,
      uint8_t* buffer,
      unsigned int buffer_size)
    {
        sgx_status_t sgxReturnValue;
        unsigned int returnValue;
        if ((sgxReturnValue = ocall_faasm_await_call_output(
               &returnValue, call_id, buffer, buffer_size)) != SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }
        return returnValue;
    }

    // -------------------------------------------
    // PTHREADS
    // 28/06/21 - WAMR-SGX threading not implemented
    // -------------------------------------------

    static int32_t pthread_mutex_init_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a,
                                              int32_t b)
    {
        return 0;
    }

    static int32_t pthread_mutex_lock_wrapper(wasm_exec_env_t exec_env,
                                              int32_t a)
    {
        return 0;
    }

    static int32_t pthread_mutex_unlock_wrapper(wasm_exec_env_t exec_env,
                                                int32_t a)
    {
        return 0;
    }

    static int32_t pthread_cond_broadcast_wrapper(wasm_exec_env_t exec_env,
                                                  int32_t a)
    {
        return 0;
    }

    static int32_t pthread_mutexattr_init_wrapper(wasm_exec_env_t exec_env,
                                                  int32_t a)
    {
        return 0;
    }

    static int32_t pthread_mutexattr_destroy_wrapper(wasm_exec_env_t exec_env,
                                                     int32_t a)
    {
        return 0;
    }

    static int32_t pthread_equal_wrapper(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int32_t b)
    {
        return 0;
    }

    // ------------------------------
    // MEMORY
    // ------------------------------

    static int32_t __sbrk_wrapper(wasm_exec_env_t exec_env, int32_t increment)
    {
        int32_t returnValue;
        sgx_status_t sgxReturnValue = ocall_sbrk(&returnValue, increment);

        if ((sgxReturnValue = ocall_sbrk(&returnValue, increment)) !=
            SGX_SUCCESS) {
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
        }

        return returnValue;
    }

    // ------------------------------
    // WASI default wrapper
    // ------------------------------

    static int args_get_wrapper(wasm_exec_env_t exec_env, int a, int b)
    {
        return 0;
    }

    static int args_sizes_get_wrapper(wasm_exec_env_t exec_env, int a, int b)
    {
        return 0;
    }

    static int fd_close_wrapper(wasm_exec_env_t exec_env, int a) { return 0; }

    static int fd_seek_wrapper(wasm_exec_env_t exec_env,
                               int a,
                               int64 b,
                               int c,
                               int d)
    {
        return 0;
    }

    static int fd_write_wrapper(wasm_exec_env_t exec_env,
                                int a,
                                int b,
                                int c,
                                int d)
    {
        return 0;
    }

    static int fd_fdstat_get_wrapper(wasm_exec_env_t exec_env, int a, int b)
    {
        return 0;
    }

    static void proc_exit_wrapper(wasm_exec_env_t exec_env, int returnCode) {}

    NativeSymbol faasm_sgx_native_symbols[FAASM_SGX_NATIVE_SYMBOLS_LEN] = {
        NATIVE_FUNC(faasm_read_input, "($i)i"),
        NATIVE_FUNC(faasm_write_output, "($i)"),
        NATIVE_FUNC(faasm_read_state, "($$i)i"),
        NATIVE_FUNC(faasm_read_state_offset, "($ii$i)i"),
        NATIVE_FUNC(faasm_write_state, "($$i)"),
        NATIVE_FUNC(faasm_write_state_offset, "($ii$i)"),
        NATIVE_FUNC(faasm_push_state, "($)"),
        NATIVE_FUNC(faasm_push_state_partial, "($)"),
        NATIVE_FUNC(faasm_push_state_partial_mask, "($$)"),
        NATIVE_FUNC(faasm_pull_state, "($i)"),
        NATIVE_FUNC(faasm_lock_state_global, "($)"),
        NATIVE_FUNC(faasm_unlock_state_global, "($)"),
        NATIVE_FUNC(faasm_lock_state_read, "($)"),
        NATIVE_FUNC(faasm_unlock_state_read, "($)"),
        NATIVE_FUNC(faasm_lock_state_write, "($)"),
        NATIVE_FUNC(faasm_unlock_state_write, "($)"),
        NATIVE_FUNC(faasm_append_state, "($$i)"),
        NATIVE_FUNC(faasm_read_appended_state, "($$ii)"),
        NATIVE_FUNC(faasm_clear_appended_state, "($)"),
        NATIVE_FUNC(faasm_flag_state_dirty, "($i)"),
        NATIVE_FUNC(faasm_flag_state_offset_dirty, "($iii)"),
        NATIVE_FUNC(faasm_chain_name, "($$i)i"),
        NATIVE_FUNC(faasm_chain_ptr, "(*$i)i"),
        NATIVE_FUNC(faasm_await_call, "(i)i"),
        NATIVE_FUNC(faasm_await_call_output, "(i)i"),
        PTHREAD_NATIVE_FUNC(pthread_mutex_init, "(ii)i"),
        PTHREAD_NATIVE_FUNC(pthread_mutex_lock, "(i)i"),
        PTHREAD_NATIVE_FUNC(pthread_mutex_unlock, "(i)i"),
        PTHREAD_NATIVE_FUNC(pthread_cond_broadcast, "(i)i"),
        PTHREAD_NATIVE_FUNC(pthread_mutexattr_init, "(i)i"),
        PTHREAD_NATIVE_FUNC(pthread_mutexattr_destroy, "(i)i"),
        PTHREAD_NATIVE_FUNC(pthread_equal, "(ii)i"),
        MEMORY_NATIVE_FUNC(__sbrk, "(i)i"),
    };

    NativeSymbol faasm_sgx_wasi_symbols[FAASM_SGX_WASI_SYMBOLS_LEN] = {
        WASI_NATIVE_FUNC(args_get, "(ii)i"),
        WASI_NATIVE_FUNC(args_sizes_get, "(ii)i"),
        WASI_NATIVE_FUNC(fd_close, "(i)i"),
        WASI_NATIVE_FUNC(fd_seek, "(iIii)i"),
        WASI_NATIVE_FUNC(fd_write, "(iiii)i"),
        WASI_NATIVE_FUNC(fd_fdstat_get, "(ii)i"),
        WASI_NATIVE_FUNC(proc_exit, "(i)"),
    };
}
