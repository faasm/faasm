//
// Created by Joshua Heinemann on 09.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>
void FAASM_FUNC(func_2,2){
    uint8_t input_buffer[2], temp;
    faasmGetInput(input_buffer,sizeof(input_buffer));
    printf("[Info/func_2] Input was %d and %d\n Switching positions\n",input_buffer[0], input_buffer[1]);
    temp = input_buffer[0];
    input_buffer[0] = input_buffer[1];
    input_buffer[1] = temp;
    faasmSetOutput(input_buffer,sizeof(input_buffer));
    return;
}
void FAASM_FUNC(func_1,1){
    printf("[Info/func_1] Hello World from func_1\n");
    return;
}
FAASM_MAIN(){
    unsigned int call_id;
    uint8_t func_2_input[] = {0xFF, 0x00};
    printf("[Info/main_] Calling func_1 without await.\n");
    call_id = faasmChainThisInput(1,0x0,0x0);
    printf("[Info/main_] Used call_id: %d\n",call_id);
    printf("[Info/main_] Now calling func_1 with await\n");
    call_id = faasmChainThisInput(1,0x0,0x0);
    faasmAwaitCall(call_id);
    printf("[Info/main_] Used call_id: %d\n",call_id);
    printf("[Info/main_] Calling func_2 with payload %d %d\n", func_2_input[0], func_2_input[1]);
    call_id = faasmChainThisInput(2,func_2_input,sizeof(func_2_input));
    faasmAwaitCallOutput(call_id,func_2_input,sizeof(func_2_input));
    printf("[Info/main_] Return value: %d %d \n",func_2_input[0],func_2_input[1]);
    printf("[Info/main_] Finished\n");
    faasmSetOutput((uint8_t*)"Execution finished",sizeof("Execution finished"));
    return;
}