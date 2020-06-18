//
// Created by Joshua Heinemann on 17.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx_wamr_native_symbols_wrapper.h>

int puts_wrapper(wasm_exec_env_t exec_env, const char* msg){
    sgx_status_t sgx_ret_val;
    int ret_val = 0;
    if((sgx_ret_val = ocall_puts(&ret_val,msg)) != SGX_SUCCESS){
        //ERROR HANDLING
    }
    return ret_val;
}