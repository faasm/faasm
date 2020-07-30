//
// Created by Joshua Heinemann on 09.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>
void func_2(void){
    printf("[Info/func_2] Hello World from func_2\n");
    return;
}
void FAASM_FUNC(func_1,1){
    printf("[Info/func_1] Hello World from func_1\n");
    return;
}
FAASM_MAIN(){
    unsigned int call_id;
    printf("[Info] chaining_external_func_sgx_wamr invoked\n");
    printf("[Info] Calling func_1 with await\n");
    call_id = faasmChainThisInput(1,0x0,0x0);
    faasmAwaitCall(call_id);
    printf("[Info] Calling internal func func_2\n");
    func_2();
    return;
}