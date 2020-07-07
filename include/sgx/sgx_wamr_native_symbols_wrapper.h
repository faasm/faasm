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
    extern sgx_status_t SGX_CDECL ocall_sgx_wamr_example_native_symbol(const char* message);
    extern sgx_status_t SGX_CDECL ocall_faasm_chain_function_input(unsigned int* ret_val, const char* name, const uint8_t* input, long input_size);
    extern sgx_status_t SGX_CDECL ocall_faasm_chain_this_input(unsigned int* ret_val, const int idx, const uint8_t* input, long input_size);
    extern sgx_status_t SGX_CDECL ocall_faasm_await_call(unsigned int* ret_val, unsigned int call_id);
    static void sgx_wamr_example_native_symbol_wrapper(wasm_exec_env_t exec_env, const char* message);
    static void faasmGetCurrentIdx(wasm_exec_env_t exec_env);
    static unsigned int faasm_chain_function_input(wasm_exec_env_t exec_env, const char* name, const uint8_t* input, long input_size);
    static unsigned int faasm_chain_this_input_wrapper(wasm_exec_env_t exec_env, int idx, const uint8_t* input, long input_size);
    static unsigned int faasm_await_call(wasm_exec_env_t exec_env, unsigned int call_id);
    uint32_t get_sgx_wamr_native_symbols(NativeSymbol** native_symbol_ptr);
};

#endif //FAASM_SGX_WAMR_NATIVE_SYMBOLS_WRAPPER_H
