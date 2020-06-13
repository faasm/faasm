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
#include <storage/FileSystem.h>
#include <storage/FileLoader.h>
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
    int sgx_launch_token_updated = 0;
    sgx_status_t enclave_ret_val;
    //!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!///
    faasm_sgx_status_t ret_val;
    std::string user = "wamr";
    std::string function = "hello_world";
    message::Message msg = util::messageFactory(user, function);
    storage::FileSystem fs;
    storage::FileLoader &fl = storage::getFileLoader();
    std::vector<uint8_t> wasm_opcode;
    uint32_t dummy_argv[] = {
            0x0,
            0x0
    };
    fs.prepareFilesystem();
    wasm_opcode = fl.loadFunctionWasm(msg);
    printf("Wasm Opcode Size: %d\n", wasm_opcode.size());
    if((enclave_ret_val = sgx_create_enclave(SGX_RUNNER_ENCLAVE_PATH,SGX_DEBUG_FLAG,&enclave_token,&sgx_launch_token_updated,&enclave_id,NULL)) != SGX_SUCCESS){
        printf("[Error] Unable to create enclave (%d)\n",enclave_ret_val);
        return -1;
    }
    if((enclave_ret_val = enclave_init_wamr(enclave_id,&ret_val)) != SGX_SUCCESS){
        CANT_ENTER_ENCLAVE:
        printf("[Error] Unable to enter enclave (%d)\n",enclave_ret_val);
        return -1;
    }
    if(ret_val != FAASM_SGX_SUCCESS){
        printf("[Error] Can't initialize WAMR RTE (%d)\n",ret_val);
        return -1;
    }
    if((enclave_ret_val = enclave_load_module(enclave_id,&ret_val,(void*)wasm_opcode.data(), wasm_opcode.size())) != SGX_SUCCESS)
        goto CANT_ENTER_ENCLAVE;
    if(ret_val != FAASM_SGX_SUCCESS){
        printf("[Error] Can't load wasm module (%d)\n",ret_val);
        return -1;
    }
    if((enclave_ret_val = enclave_call_function(enclave_id,&ret_val,"main",2,dummy_argv)) != SGX_SUCCESS){
        goto CANT_ENTER_ENCLAVE;
    }
    if(ret_val != FAASM_SGX_SUCCESS){
        printf("[Error] Unable to call desired function (%d)\n",ret_val);
        return -1;
    }
    //!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!///
}