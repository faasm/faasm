//
// Created by Joshua Heinemann on 09.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>

void FAASM_FUNC(func_1,1){
    uint32_t input_size;
    uint8_t buffer[32];
    printf("[Info/func_1] Reading input\n");
    input_size = faasmGetInputSize();
    printf("[Info/func_1] Input size: %u\n",input_size);
    faasmGetInput(buffer,sizeof(buffer));
    printf("[Info/func_1] Input: %s\n",(char*)buffer);
    return;
}
void FAASM_MAIN(main_){
    char func_1_input[] = {"Hello"};
    unsigned int call_id;
    printf("[Info/main_] Calling func_1 with input %s\n",func_1_input);
    call_id = faasmChainThisInput(1,(const uint8_t*)func_1_input,sizeof(func_1_input));
    faasmAwaitCall(call_id);
    printf("[Info/main_] Finished\n");
    return;
}