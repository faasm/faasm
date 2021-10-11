#include <sgx/native.h>

namespace sgx {
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
           key, total_len, offset, buffer_ptr, buffer_len)) != SGX_SUCCESS) {
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
           key, total_len, offset, buffer_ptr, buffer_len)) != SGX_SUCCESS) {
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

static void faasm_push_state_wrapper(wasm_exec_env_t exec_env, const char* key)
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
    if ((sgxReturnValue = ocall_faasm_push_state_partial(key)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
}

static void faasm_push_state_partial_mask_wrapper(wasm_exec_env_t exec_env,
                                                  const char* key,
                                                  const char* mask_key)
{
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocall_faasm_push_state_partial_mask(key, mask_key)) !=
        SGX_SUCCESS) {
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
    if ((sgxReturnValue = ocall_faasm_lock_state_global(key)) != SGX_SUCCESS) {
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
    if ((sgxReturnValue = ocall_faasm_lock_state_read(key)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
}

static void faasm_unlock_state_read_wrapper(wasm_exec_env_t exec_env,
                                            const char* key)
{
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocall_faasm_unlock_state_read(key)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
}

static void faasm_lock_state_write_wrapper(wasm_exec_env_t exec_env,
                                           const char* key)
{
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocall_faasm_lock_state_write(key)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
}

static void faasm_unlock_state_write_wrapper(wasm_exec_env_t exec_env,
                                             const char* key)
{
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocall_faasm_unlock_state_write(key)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }
}
static NativeSymbol ns[] = {
    REG_FAASM_NATIVE_FUNC(faasm_read_state, "($$i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_read_state_offset, "($ii$i)i"),
    REG_FAASM_NATIVE_FUNC(faasm_write_state, "($$i)"),
    REG_FAASM_NATIVE_FUNC(faasm_write_state_offset, "($ii$i)"),
    REG_FAASM_NATIVE_FUNC(faasm_push_state, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_push_state_partial, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_push_state_partial_mask, "($$)"),
    REG_FAASM_NATIVE_FUNC(faasm_pull_state, "($i)"),
    REG_FAASM_NATIVE_FUNC(faasm_lock_state_global, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_unlock_state_global, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_lock_state_read, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_unlock_state_read, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_lock_state_write, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_unlock_state_write, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_append_state, "($$i)"),
    REG_FAASM_NATIVE_FUNC(faasm_read_appended_state, "($$ii)"),
    REG_FAASM_NATIVE_FUNC(faasm_clear_appended_state, "($)"),
    REG_FAASM_NATIVE_FUNC(faasm_flag_state_dirty, "($i)"),
    REG_FAASM_NATIVE_FUNC(faasm_flag_state_offset_dirty, "($iii)"),
};

uint32_t getFaasmStateApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
