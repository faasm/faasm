//
// Created by Joshua Heinemann on 26.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx/faasm_sgx_wamr.h>
#include <stdio.h>

#define STATE_KEY "my_secret"
#define SECRET_MESSAGE "Hello World"

FAASM_MAIN(){
    uint8_t buffer_ptr[sizeof(SECRET_MESSAGE)];
    uint32_t secret_size;
    printf("[Info/main_] Placing secret message %s into a state\n", SECRET_MESSAGE);
    faasmWriteState(STATE_KEY,(uint8_t*)SECRET_MESSAGE, sizeof(SECRET_MESSAGE));
    printf("[Info/main_] Now fetching the secret\n");
    secret_size = faasmReadStateSize(STATE_KEY);
    if(secret_size != sizeof(SECRET_MESSAGE)){
        printf("[Error/main_] Wrong state size\n");
        return;
    }
    faasmReadState(STATE_KEY,buffer_ptr, sizeof(SECRET_MESSAGE));
    printf("[Info/main_] Placed secret message was: %s (size %u)\n", (char*) buffer_ptr, secret_size);
    return;
}