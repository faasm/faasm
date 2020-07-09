//
// Created by Joshua Heinemann on 09.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>
void FAASM_FUNC(func_1,1){
    printf("[Info/func_1] Hello World from func_1\n");

    return;
}
void FAASM_MAIN(main_){
    unsigned int call_id;
    printf("[Info/main_] Calling func_1 without await.\n");
    call_id = faasmChainThisInput(1,0x0,0x0);
    printf("[Info/main_] Used call_id: %d\n",call_id);
    printf("[Info/main_] Now calling func_1 with await\n");
    call_id = faasmChainThisInput(1,0x0,0x0);
    faasmAwaitCall(call_id);
    printf("[Info/main_] Used call_id: %d\n",call_id);
    printf("[Info/main_] Finished\n");
    return;
}