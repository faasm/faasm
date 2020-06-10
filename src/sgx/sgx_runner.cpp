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
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

extern "C"{
void ocall_printf(const char* message){
    printf("[Enclave] %s",message);
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
    int fd;
    struct stat hello_world_fstat;
    faasm_sgx_status_t ret_val;
    memset(&hello_world_fstat,0x0,sizeof(struct stat));
    if((fd = open("bin/hello_world.wasm",O_RDONLY)) == -1){
        perror("open");
        return -1;
    }
    fstat(fd,&hello_world_fstat);
    void* hello_world_ptr = NULL;
    if((hello_world_ptr = mmap(NULL,hello_world_fstat.st_size,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE,fd,0x0)) == (void*)-1){
        perror("mmap");
        return -1;
    }
    printf("[Info] Hello World from sgx_runner\n");
    if((enclave_ret_val = sgx_create_enclave(SGX_RUNNER_ENCLAVE_PATH,SGX_DEBUG_FLAG,&enclave_token,&sgx_launch_token_updated,&enclave_id,NULL)) != SGX_SUCCESS){
        printf("[Error] Unable to create enclave (%d).\n",enclave_ret_val);
        return 0;
    }
    if((enclave_ret_val = enclave_init_wamr(enclave_id,&ret_val)) != SGX_SUCCESS){
        printf("[Error] enclave_init_wamr (%d)\n",enclave_ret_val);
        return -1;
    }
    if(ret_val != FAASM_SGX_SUCCESS){
        printf("GRande error\n");
    }
    if((enclave_ret_val = enclave_load_module(enclave_id,&ret_val,hello_world_ptr,hello_world_fstat.st_size)) != SGX_SUCCESS){
        printf("[Error] Unable to enter enclave (%d).\n",enclave_ret_val);
        return -1;
    }
    if(ret_val != FAASM_SGX_SUCCESS){
        printf("Error %d\n",ret_val);
        return -1;
    }
    uint32_t func_parameter[] ={
            {NULL},
            {NULL}
    };
    if((enclave_ret_val = enclave_call_function(enclave_id,&ret_val,"main",2,func_parameter)) != SGX_SUCCESS){
        printf("[Error] Unable to enter enclave (%d).\n",enclave_ret_val);
    }
    if(ret_val != FAASM_SGX_SUCCESS){
        printf("Error %d\n",ret_val);
        return -1;
    }
    return 0;
    //!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!/////!!! TEST CODE !!!///
}