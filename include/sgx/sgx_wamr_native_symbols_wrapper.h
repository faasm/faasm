#pragma once

#include <sgx.h>
#include <sgx_defs.h>
#include <iwasm/include/wasm_export.h>

extern "C" {

// --------------------------------------
// OCALLS
// --------------------------------------

extern sgx_status_t SGX_CDECL ocall_faasm_read_state_size(
        uint64_t *ret_val, const char *key
);

extern sgx_status_t SGX_CDECL ocall_faasm_read_state(
        uint64_t *ret_val, const char *key, uint8_t *buffer, const uint32_t buffer_len
);

extern sgx_status_t SGX_CDECL ocall_faasm_read_appended_state(
        const char *key, uint8_t *buffer_ptr, const uint32_t buffer_len, const uint32_t element_num
);

extern sgx_status_t SGX_CDECL ocall_faasm_read_state_offset(
        const char *key, const uint64_t total_len, const uint64_t offset, uint8_t *buffer_ptr, const uint32_t buffer_len
);

extern sgx_status_t SGX_CDECL ocall_faasm_write_state(
        const char *key, const uint8_t *buffer_ptr, const uint32_t buffer_len
);

extern sgx_status_t SGX_CDECL ocall_faasm_append_state(
        const char *key, const uint8_t *buffer_ptr, const uint32_t buffer_len
);

extern sgx_status_t SGX_CDECL ocall_faasm_clear_appended_state(const char *key);

extern sgx_status_t SGX_CDECL
ocall_faasm_write_state_offset(
        const char *key, const uint64_t total_len, const uint64_t offset,
        const uint8_t *buffer_ptr, const uint32_t buffer_len
);

extern sgx_status_t SGX_CDECL ocall_faasm_flag_state_dirty(
        const char *key, const uint64_t total_len
);

extern sgx_status_t SGX_CDECL ocall_faasm_flag_state_offset_dirty(
        const char *key, const uint64_t total_len, const uint64_t offset, const uint64_t len
);

extern sgx_status_t SGX_CDECL ocall_faasm_push_state(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_push_state_partial(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_push_state_partial_mask(const char *key, const char *mask_key);

extern sgx_status_t SGX_CDECL ocall_faasm_pull_state(const char *key, const uint64_t state_len);

extern sgx_status_t SGX_CDECL ocall_faasm_lock_state_global(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_unlock_state_global(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_lock_state_read(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_unlock_state_read(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_lock_state_write(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_unlock_state_write(const char *key);

extern sgx_status_t SGX_CDECL ocall_faasm_get_input_size(unsigned int *ret_val);

extern sgx_status_t SGX_CDECL ocall_faasm_get_input(uint8_t *buffer, unsigned int buffer_size);

extern sgx_status_t SGX_CDECL ocall_faasm_set_output(uint8_t *output, unsigned int output_size);

extern sgx_status_t SGX_CDECL ocall_faasm_chain_function_input(
        unsigned int *ret_val, const char *name, const uint8_t *input, unsigned int input_size
);

extern sgx_status_t SGX_CDECL ocall_faasm_chain_this_input(
        unsigned int *ret_val, const int idx, const uint8_t *input, unsigned int input_size
);

extern sgx_status_t SGX_CDECL ocall_faasm_await_call(
        unsigned int *ret_val, unsigned int call_id
);

extern sgx_status_t SGX_CDECL ocall_faasm_await_call_output(
        unsigned int *ret_val, unsigned int call_id, uint8_t *buffer, unsigned int buffer_size
);

extern sgx_status_t SGX_CDECL ocall_faasm_get_current_idx(int *ret_val);

// --------------------------------------
// FAASM HOST INTERFACE
// --------------------------------------

static uint64_t faasm_read_state_size_wrapper(wasm_exec_env_t exec_env, const char *key);

static uint64_t faasm_read_state_wrapper(
        wasm_exec_env_t exec_env, const char *key, uint8_t *buffer_ptr, const uint32_t buffer_len
);

static void faasm_read_appended_state_wrapper(
        wasm_exec_env_t exec_env, const char *key, uint8_t *buffer_ptr,
        const uint32_t buffer_len, const uint32_t element_num
);

static void faasm_read_state_offset_wrapper(
        wasm_exec_env_t exec_env, const char *key, const uint64_t total_len,
        const uint64_t offset, uint8_t *buffer_ptr, const uint32_t buffer_len
);

static void faasm_write_state_wrapper(
        wasm_exec_env_t exec_env, const char *key, const uint8_t *buffer_ptr,
        const uint32_t buffer_len
);

static void faasm_append_state_wrapper(
        wasm_exec_env_t exec_env, const char *key, const uint8_t *buffer_ptr,
        const uint32_t buffer_len
);

static void faasm_clear_appended_state_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_write_state_offset_wrapper(
        wasm_exec_env_t exec_env, const char *key, const uint64_t total_len,
        const uint64_t offset, const uint8_t *buffer_ptr,
        const uint32_t buffer_len
);

static void faasm_flag_state_dirty_wrapper(wasm_exec_env_t exec_env, const char *key, const uint64_t total_len);

static void faasm_flag_state_offset_dirty_wrapper(
        wasm_exec_env_t exec_env, const char *key, const uint64_t total_len, const uint64_t offset, const uint64_t len
);

static void faasm_push_state_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_push_state_partial_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_push_state_partial_mask_wrapper(wasm_exec_env_t exec_env, const char *key, const char *mask_key);

static void faasm_pull_state_wrapper(wasm_exec_env_t exec_env, const char *key, const uint64_t state_len);

static void faasm_lock_state_global_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_unlock_state_global_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_lock_state_read_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_unlock_state_read_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_lock_state_write_wrapper(wasm_exec_env_t exec_env, const char *key);

static void faasm_unlock_state_write_wrapper(wasm_exec_env_t exec_env, const char *key);

static unsigned int faasm_get_input_size_wrapper(wasm_exec_env_t exec_env);

static void faasm_get_input_wrapper(wasm_exec_env_t exec_env, uint8_t *buffer, unsigned int buffer_size);

static void faasm_set_output_wrapper(wasm_exec_env_t exec_env, uint8_t *output, unsigned int output_size);

static unsigned int faasm_chain_function_input_wrapper(
        wasm_exec_env_t exec_env, const char *name, const uint8_t *input, unsigned int input_size
);

static unsigned int faasm_chain_this_input_wrapper(
        wasm_exec_env_t exec_env, int idx, const uint8_t *input, unsigned int input_size
);

static unsigned int faasm_await_call_wrapper(wasm_exec_env_t exec_env, unsigned int call_id);

static unsigned int faasm_await_call_output_wrapper(
        wasm_exec_env_t exec_env, unsigned int call_id, uint8_t *buffer, unsigned int buffer_size
);

static int faasm_get_current_idx_wrapper(wasm_exec_env_t exec_env);

// --------------------------------------
// WHITELISTING
// --------------------------------------

void sgx_wamr_function_not_whitelisted_wrapper(wasm_exec_env_t exec_env);

}
