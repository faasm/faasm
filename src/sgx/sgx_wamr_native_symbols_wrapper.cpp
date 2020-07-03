//
// Created by Joshua Heinemann on 17.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx_wamr_native_symbols_wrapper.h>

extern "C"{
    /*static void myputs_wrapper(wasm_exec_env_t exec_env, const char* message){
        ocall_puts(message);
    }*/
    static NativeSymbol sgx_wamr_native_symbols[] = {
            //{"myputs",(void*)myputs_wrapper,"($)",0x0}
    };
    uint32_t get_sgx_wamr_native_symbols(NativeSymbol** native_symbol_ptr){
        *native_symbol_ptr = sgx_wamr_native_symbols;
        return sizeof(sgx_wamr_native_symbols)/sizeof(NativeSymbol);
    }
};
