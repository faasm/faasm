//
// Created by Joshua Heinemann on 05.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>

void bar(void){
    printf("[Info] Bar\n");
    return;
}
void FAASM_FUNC(read_input, 2){
    printf("[Info] Still not implemented.\n");
}
void FAASM_FUNC(foo, 1){
    printf("[Info] Foo\n");
    bar();
    return;
}
void FAASM_MAIN(main_x){
    unsigned int call_id;
    printf("[Info] Create chained call without await call\n");
    call_id = faasmChainThisInput(1,0x0,0x0);
    foo();
    printf("[Info] Create chained call with await call\n");
    call_id = faasmChainThisInput(1,0x0,0x0);
    printf("[Info] await return value %d\n",faasmAwaitCall(call_id));
    foo();
    uint8_t payload[] = {0x0,0x1,0x2};
    call_id = faasmChainThisInput(2,payload,sizeof(payload));
    printf("[Info] Calling hello_world_sgx_wamr\n");
    call_id = faasmChainFunctionInput("hello_world_sgx_wamr",0x0,0x0);
    printf("Call id %d\n",call_id);
    //faasmAwaitCall(call_id);
    printf("[Info]hello_world_sgx_wamr finished\n");
    return;
}