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
#include <sgx/faasm_sgx_error.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <util/func.h>

extern "C"{
void ocall_printf(const char* message){
    printf("%s",message);
}
extern sgx_status_t enclave_call_function(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const char* wasm_function_name, uint32_t wasm_function_argc, uint32_t* wasm_function_argv);
extern sgx_status_t enclave_load_module(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, void* wasm_opcode_ptr, uint32_t wasm_opcode_size);
extern sgx_status_t enclave_init_wamr(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val);
};
int main(int argc, char** argv){
    sgx_enclave_id_t enclave_id;
    sgx_launch_token_t enclave_token = {0};
    sgx_status_t sgx_ret_val;
    int enclave_token_updated = 0;
    message::Message msg;
    wasm::SGXWAMRWasmModule module(&enclave_id);
    if(argc < 3){
        printf("[Error] Too few arguments. Please enter user and function\n");
        return -1;
    }
    msg = util::messageFactory(argv[1],argv[2]);
    if((sgx_ret_val = sgx_create_enclave(SGX_RUNNER_ENCLAVE_PATH,SGX_DEBUG_FLAG,&enclave_token,&enclave_token_updated,&enclave_id,NULL)) != SGX_SUCCESS){
        printf("[Error] Unable to create enclave (%d)\n", sgx_ret_val);
        return -1;
    }
    module.bindToFunction(msg);
    module.execute(msg);
}