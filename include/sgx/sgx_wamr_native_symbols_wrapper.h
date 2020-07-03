//
// Created by Joshua Heinemann on 18.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//
#ifndef FAASM_SGX_WAMR_NATIVE_SYMBOLS_WRAPPER_H
#define FAASM_SGX_WAMR_NATIVE_SYMBOLS_WRAPPER_H

#include <sgx.h>
#include <sgx_defs.h>
#include <wasm_export.h>

extern "C"{
    extern sgx_status_t SGX_CDECL ocall_puts(const char* message);
    //static void myputs_wrapper(wasm_exec_env_t exec_env, const char* message);
    uint32_t get_sgx_wamr_native_symbols(NativeSymbol** native_symbol_ptr);
};

#endif //FAASM_SGX_WAMR_NATIVE_SYMBOLS_WRAPPER_H
