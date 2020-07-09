//
// Created by Joshua Heinemann on 09.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>

void FAASM_FUNC(add,1){
    uint8_t buffer[2];
    faasmGetInput(buffer,sizeof(buffer));
    printf("[Info/add] Reading input values\n");
    printf("[Info/add] Calculating %d + %d\n",buffer[0], buffer[1]);
    buffer[0] = buffer[0] + buffer[1];
    printf("[Info/add] Calculation completed\n");
    faasmSetOutput(buffer,1);
    return;
}
void FAASM_MAIN(main_){
    uint32_t call_id;
    uint8_t input[2] = {0x1, 0x2};
    printf("[Info/main_] Invoking add() with input values 1 & 2\n");
    call_id = faasmChainThisInput(1,input, sizeof(input));
    faasmAwaitCallOutput(call_id,input,sizeof(input));
    printf("[Info/main_] Result: %d\n",input[0]);
    printf("[Info/main_] Finished\n");
}