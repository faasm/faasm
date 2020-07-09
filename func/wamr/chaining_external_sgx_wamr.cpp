//
// Created by Joshua Heinemann on 09.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>

void FAASM_MAIN(main_){
    unsigned int call_id;
    printf("[Info/main_] Calling chaining_external_func_sgx_wamr\n");
    call_id = faasmChainFunctionInput("chaining_external_func_sgx_wamr",0x0,0x0);
    faasmAwaitCall(call_id);
    printf("[Info/main_] Calling hello_world_sgx_wamr without await\n");
    call_id = faasmChainFunctionInput("hello_world_sgx_wamr",0x0,0x0);
    printf("[Info] Finished\n");
}