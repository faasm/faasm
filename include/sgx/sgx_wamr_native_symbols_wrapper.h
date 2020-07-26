//
// Created by Joshua Heinemann on 18.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//
#ifndef FAASM_SGX_WAMR_NATIVE_SYMBOLS_WRAPPER_H
#define FAASM_SGX_WAMR_NATIVE_SYMBOLS_WRAPPER_H

#include <sgx.h>
#include <sgx_defs.h>
#include <iwasm/include/wasm_export.h>

extern "C"{
    extern sgx_status_t SGX_CDECL ocall_faasm_read_state_size(uint64_t* ret_val, const char* key);
    extern sgx_status_t SGX_CDECL ocall_faasm_read_state(uint64_t* ret_val, const char* key, uint8_t* buffer, const uint32_t buffer_len);
    extern sgx_status_t SGX_CDECL ocall_faasm_write_state(const char* key, const uint8_t* buffer_ptr, const uint32_t buffer_len);
    extern sgx_status_t SGX_CDECL ocall_faasm_get_input_size(unsigned int* ret_val);
    extern sgx_status_t SGX_CDECL ocall_faasm_get_input(uint8_t* buffer, unsigned int buffer_size);
    extern sgx_status_t SGX_CDECL ocall_faasm_set_output(uint8_t* output, unsigned int output_size);
    extern sgx_status_t SGX_CDECL ocall_faasm_chain_function_input(unsigned int* ret_val, const char* name, const uint8_t* input, unsigned int input_size);
    extern sgx_status_t SGX_CDECL ocall_faasm_chain_this_input(unsigned int* ret_val, const int idx, const uint8_t* input, unsigned int input_size);
    extern sgx_status_t SGX_CDECL ocall_faasm_await_call(unsigned int* ret_val, unsigned int call_id);
    extern sgx_status_t SGX_CDECL ocall_faasm_await_call_output(unsigned int* ret_val, unsigned int call_id, uint8_t* buffer, unsigned int buffer_size);
    static uint64_t faasm_read_state_size_wrapper(wasm_exec_env_t exec_env, const char* key);
    static uint64_t faasm_read_state_wrapper(wasm_exec_env_t exec_env, const char* key, uint8_t* buffer_ptr, uint32_t buffer_len);
    static void faasm_write_state_wrapper(wasm_exec_env_t exec_env, const char* key, const uint8_t* buffer_ptr, const uint32_t buffer_len);
    static unsigned int faasm_get_input_size_wrapper(wasm_exec_env_t exec_env);
    static void faasm_get_input_wrapper(wasm_exec_env_t exec_env, uint8_t* buffer, unsigned int buffer_size);
    static void faasm_set_output_wrapper(wasm_exec_env_t exec_env, uint8_t* output, unsigned int output_size);
    static unsigned int faasm_chain_function_input_wrapper(wasm_exec_env_t exec_env, const char* name, const uint8_t* input, unsigned int input_size);
    static unsigned int faasm_chain_this_input_wrapper(wasm_exec_env_t exec_env, int idx, const uint8_t* input, unsigned int input_size);
    static unsigned int faasm_await_call_wrapper(wasm_exec_env_t exec_env, unsigned int call_id);
    static unsigned int faasm_await_call_output_wrapper(wasm_exec_env_t exec_env, unsigned int call_id, uint8_t* buffer, unsigned int buffer_size);
    uint32_t get_sgx_wamr_native_symbols(NativeSymbol** native_symbol_ptr);
};

#endif //FAASM_SGX_WAMR_NATIVE_SYMBOLS_WRAPPER_H
