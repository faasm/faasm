//
// Created by Joshua Heinemann on 04.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#ifndef SGX_RUNNER_ENCLAVE_PATH
#define SGX_RUNNER_ENCLAVE_PATH "bin/sgx_runner_enclave.sign.so"
#endif

#include <stdio.h>
#include <sgx.h>
#include <sgx_urts.h>
extern "C"{
void ocall_printf(char* message){
    printf("[Enclave] %s\n",message);
}
extern sgx_status_t enclave_main(sgx_enclave_id_t);
};
int main(int argc, char** argv){
    sgx_enclave_id_t enclave_id;
    sgx_launch_token_t enclave_token = {0};
    int sgx_launch_token_updated = 0;
    sgx_status_t enclave_ret_val;
    printf("[Info] Hello World from sgx_runner\n");

    if((enclave_ret_val = sgx_create_enclave(SGX_RUNNER_ENCLAVE_PATH,SGX_DEBUG_FLAG,&enclave_token,&sgx_launch_token_updated,&enclave_id,NULL)) != SGX_SUCCESS){
        printf("[Error] Unable to create enclave (%d).\n",enclave_ret_val);
        return 0;
    }
    if((enclave_ret_val = enclave_main(enclave_id)) != SGX_SUCCESS){
        printf("[Error] Unable to enter enclave (%d).\n");
    }
    return 0;
}